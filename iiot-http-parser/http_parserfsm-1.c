/********************************************************
 * http_parserfsm.c
 *
 * Implements the HTTP request parser FSM for the
 * IIoT sensor endpoint. Parses HTTP requests directly
 * from the socket receive buffer (no local copy).
 *
 * Supported endpoints:
 *   GET    /device
 *   PUT    /device?reset="true"|"false"
 *   PUT    /device/config?<param>=<value>
 *   DELETE /device/log
 *
 * Author:   Ahlaireah Lewis
 * Date:     4/30/2026
 * Revision: 1.0
 *
 ********************************************************/

#include "http_parserfsm.h"
#include "socket.h"
#include "config.h"
#include "log.h"
#include "temp.h"
#include "vpd.h"
#include "wdt.h"
#include "alarm.h"
#include "uart.h"

/**********************************************************
 * FSM STATE DEFINITIONS
 **********************************************************/
#define REQLINESEC  1
#define HEADSEC     2
#define BODYSEC     3
#define PROCESS     4

/***********************************************************
 * METHOD DEFINITIONS
 ***********************************************************/
#define METHOD_UNKNOWN  0
#define METHOD_GET      1
#define METHOD_PUT      2
#define METHOD_DELETE   3

/***********************************************************
 * URI DEFINITIONS
 ***********************************************************/
#define URI_UNKNOWN         0
#define URI_DEVICE          1   /* /device          */
#define URI_DEVICE_CONFIG   2   /* /device/config   */
#define URI_DEVICE_LOG      3   /* /device/log      */

/***********************************************************
 * PUT PARAMETER DEFINITIONS
 ***********************************************************/
#define PARAM_NONE      0
#define PARAM_TCRIT_HI  1
#define PARAM_TWARN_HI  2
#define PARAM_TCRIT_LO  3
#define PARAM_TWARN_LO  4
#define PARAM_RESET     5

/***********************************************************
 * STATIC STATE VARIABLES
 * Persist across cyclic executive calls.
 ***********************************************************/
static unsigned char state;
static unsigned char method;
static unsigned char uri;
static unsigned char param;
static int           param_value;
static unsigned char reset_flag;

/***********************************************************
 * HTTP RESPONSE HEADERS
 ***********************************************************/
static const char* HTTP_200 = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";
static const char* HTTP_400 = "HTTP/1.1 400 BAD REQUEST\r\n\r\n";

/***********************************************************
 * INTERNAL: PUT VALIDATION AND PERSISTENCE HELPERS
 ***********************************************************/

/********************************************************
 * Function: update_tcrit_hi
 * Description:
 * Updates high critical temperature threshold if valid.
 *
 * Arguments:
 *  value - new threshold value
 *
 * Returns:
 *  0 = success
 *  1 = invalid value
 *
 * Hardware:
 *  - writes to non-volatile config storage
 *
 * Notes:
 *  value must be > hi_warn and <= 10-bit max (0x3FF)
 ********************************************************/
static int update_tcrit_hi(int value) {
    if (value > config.hi_warn && value <= 0x3FF) {
        config.hi_alarm = value;
        config_update();
        return 0;
    }
    return 1;
}

/********************************************************
 * Function: update_twarn_hi
 * Description:
 * Updates high warning temperature threshold if valid.
 *
 * Arguments:
 *  value - new threshold value
 *
 * Returns:
 *  0 = success
 *  1 = invalid value
 *
 * Hardware:
 *  - writes to non-volatile config storage
 *
 * Notes:
 *  value must be > lo_warn and < hi_alarm
 ********************************************************/
static int update_twarn_hi(int value) {
    if (value > config.lo_warn && value < config.hi_alarm) {
        config.hi_warn = value;
        config_update();
        return 0;
    }
    return 1;
}

/********************************************************
 * Function: update_tcrit_lo
 * Description:
 * Updates low critical temperature threshold if valid.
 *
 * Arguments:
 *  value - new threshold value
 *
 * Returns:
 *  0 = success
 *  1 = invalid value
 *
 * Hardware:
 *  - writes to non-volatile config storage
 *
 * Notes:
 *  value must be < lo_warn
 ********************************************************/
static int update_tcrit_lo(int value) {
    if (value < config.lo_warn) {
        config.lo_alarm = value;
        config_update();
        return 0;
    }
    return 1;
}


/********************************************************
 * Function: update_twarn_lo
 * Description:
 * Updates low warning temperature threshold if valid.
 *
 * Arguments:
 *  value - new threshold value
 *
 * Returns:
 *  0 = success
 *  1 = invalid value
 *
 * Hardware:
 *  - writes to non-volatile config storage
 *
 * Notes:
 *  value must be > lo_alarm and < hi_warn
 ********************************************************/
static int update_twarn_lo(int value) {
    if (value > config.lo_alarm && value < config.hi_warn) {
        config.lo_warn = value;
        config_update();
        return 0;
    }
    return 1;
}

/********************************************************
 * Function: send_get_response
 * Description:
 * Builds and sends HTTP JSON response for /device endpoint.
 *
 * Arguments: none
 *
 * Returns: none
 *
 * Hardware:
 *  - writes directly to TCP socket buffer
 *  - reads temperature sensor
 *  - accesses log storage
 *
 * Notes:
 *  response is streamed directly (no intermediate buffer)
 *  includes VPD, config thresholds, current temp, and log
 ********************************************************/
static void send_get_response(void) {
    unsigned char i;
    unsigned char num_entries;
    unsigned long entry_time;
    unsigned char entry_event;
    int t;
    const char* state_str;

    t = temp_get();
    if      (t >= config.hi_alarm) state_str = "CRIT_HI";
    else if (t >= config.hi_warn)  state_str = "WARN_HI";
    else if (t <= config.lo_alarm) state_str = "CRIT_LO";
    else if (t <= config.lo_warn)  state_str = "WARN_LO";
    else                           state_str = "NORMAL";

    socket_writestr(0, HTTP_200);

    /* Open device object */
    socket_writestr(0, "{");

    /* VPD sub-object */
    socket_writestr(0, "\"vpd\":{");
    socket_writestr(0, "\"model\":");
    socket_writequotedstring(0, vpd.model);
    socket_writestr(0, ",\"manufacturer\":");
    socket_writequotedstring(0, vpd.manufacturer);
    socket_writestr(0, ",\"serial_number\":");
    socket_writequotedstring(0, vpd.serial_number);
    socket_writestr(0, ",\"manufacture_date\":");
    socket_writedate(0, vpd.manufacture_date);
    socket_writestr(0, ",\"mac_address\":");
    socket_write_macaddress(0, vpd.mac_address);
    socket_writestr(0, ",\"country_code\":");
    socket_writequotedstring(0, vpd.country_of_origin);
    socket_writestr(0, "}");

    /* Temperature thresholds */
    socket_writestr(0, ",\"tcrit_hi\":");
    socket_writedec32(0, config.hi_alarm);
    socket_writestr(0, ",\"twarn_hi\":");
    socket_writedec32(0, config.hi_warn);
    socket_writestr(0, ",\"tcrit_lo\":");
    socket_writedec32(0, config.lo_alarm);
    socket_writestr(0, ",\"twarn_lo\":");
    socket_writedec32(0, config.lo_warn);

    /* Current temperature and state */
    socket_writestr(0, ",\"temperature\":");
    socket_writedec32(0, t);
    socket_writestr(0, ",\"state\":");
    socket_writequotedstring(0, state_str);

    /* Log array */
    socket_writestr(0, ",\"log\":[");
    num_entries = log_get_num_entries();
    for (i = 0; i < num_entries; i++) {
        if (i > 0) socket_writestr(0, ",");
        if (log_get_record((unsigned long)i, &entry_time, &entry_event)) {
            socket_writestr(0, "{\"timestamp\":");
            socket_writedate(0, entry_time);
            socket_writestr(0, ",\"event\":");
            socket_writedec32(0, (int)entry_event);
            socket_writestr(0, "}");
        }
    }
    socket_writestr(0, "]}");
}

/********************************************************
 * Function: parse_request_line
 * Description:
 * Parses HTTP request line from socket buffer.
 *
 * Arguments: none
 *
 * Returns:
 *  1 = success
 *  0 = parse failure
 *
 * Hardware:
 *  - consumes data from socket receive buffer
 *
 * Notes:
 *  extracts method, URI, and query parameters
 *  supports GET, PUT, DELETE
 *  consumes line including CRLF on success
 ********************************************************/
static int parse_request_line(void) {
    method      = METHOD_UNKNOWN;
    uri         = URI_UNKNOWN;
    param       = PARAM_NONE;
    param_value = 0;
    reset_flag  = 0;

    /* Match method token */
    if      (socket_recv_compare(0, "GET "))    method = METHOD_GET;
    else if (socket_recv_compare(0, "PUT "))    method = METHOD_PUT;
    else if (socket_recv_compare(0, "DELETE ")) method = METHOD_DELETE;
    else    return 0;

    /* Match URI — longest prefix first */
    if      (socket_recv_compare(0, "/device/config")) uri = URI_DEVICE_CONFIG;
    else if (socket_recv_compare(0, "/device/log"))    uri = URI_DEVICE_LOG;
    else if (socket_recv_compare(0, "/device"))        uri = URI_DEVICE;
    else    return 0;

    /* Validate method + URI combination */
    if (uri == URI_DEVICE        && method == METHOD_DELETE) return 0;
    if (uri == URI_DEVICE_CONFIG && method != METHOD_PUT)    return 0;
    if (uri == URI_DEVICE_LOG    && method != METHOD_DELETE) return 0;

    /* Parse optional query string */
    if (socket_recv_compare(0, "?")) {

        if (uri == URI_DEVICE && method == METHOD_PUT) {
            if (!socket_recv_compare(0, "reset=")) return 0;
            param = PARAM_RESET;
            if      (socket_recv_compare(0, "\"true\""))  reset_flag = 1;
            else if (socket_recv_compare(0, "\"false\"")) reset_flag = 0;
            else    return 0;

        } else if (uri == URI_DEVICE_CONFIG && method == METHOD_PUT) {
            if      (socket_recv_compare(0, "tcrit_hi=")) param = PARAM_TCRIT_HI;
            else if (socket_recv_compare(0, "twarn_hi=")) param = PARAM_TWARN_HI;
            else if (socket_recv_compare(0, "tcrit_lo=")) param = PARAM_TCRIT_LO;
            else if (socket_recv_compare(0, "twarn_lo=")) param = PARAM_TWARN_LO;
            else    return 0;

            if (!socket_recv_int(0, &param_value)) return 0;

        } else {
            return 0; /* unexpected query string */
        }
    }

    socket_flush_line(0);
    return 1;
}

/********************************************************
 * Function: http_parserfsm_init
 * Description:
 * Initializes HTTP parser finite state machine.
 *
 * Arguments: none
 *
 * Returns: none
 *
 * Hardware: none
 *
 * Notes:
 *  resets all parser state variables
 ********************************************************/
void http_parserfsm_init(void) {
    state       = REQLINESEC;
    method      = METHOD_UNKNOWN;
    uri         = URI_UNKNOWN;
    param       = PARAM_NONE;
    param_value = 0;
    reset_flag  = 0;
}

/********************************************************
 * Function: http_parserfsm_update
 * Description:
 * Executes one step of HTTP parser FSM.
 *
 * Arguments: none
 *
 * Returns: none
 *
 * Hardware:
 *  - reads from socket buffer
 *  - writes HTTP responses to socket
 *  - may trigger watchdog reset
 *
 * Notes:
 *  should be called when a full line is available
 *  handles request parsing, execution, and response
 ********************************************************/
void http_parserfsm_update(void) {
    int result;
    // uart_writestr("FSM state: ");
    // uart_writedec32(state);
    // uart_writestr("\r\n");

    switch (state) {

            /* REQLINESEC: Wait for and parse the request line.*/
        case REQLINESEC:

            // uart_writestr(" avail: ");
            // uart_writedec32(socket_recv_available(0));
            // uart_writestr(" recvline: ");
            // uart_writedec32(socket_received_line(0));
            // uart_writestr("\r\n");

            if (!socket_received_line(0)){ 
                //uart_writestr("Waiting for full line\r\n");
                break;
            }

            if (socket_is_blank_line(0)) {
                //uart_writestr("Flushing blank line\r\n");
                socket_flush_line(0);
                socket_writestr(0, HTTP_400);
                socket_disconnect(0);
                state = REQLINESEC;
                break;
            }

            if (!parse_request_line()) {
                socket_flush_line(0);
                socket_writestr(0, HTTP_400);
                socket_disconnect(0);
                state = REQLINESEC;
                break;
            }

            state = HEADSEC;
            break;

            /*HEADSEC: Discard header lines until blank line.*/
        case HEADSEC:
            if (!socket_received_line(0)) break;

            if (socket_is_blank_line(0)) {
                socket_flush_line(0);
                state = BODYSEC;
                break;
            }

            socket_flush_line(0);
            state = BODYSEC;
            break;

            /* BODYSEC: No body expected for any endpoint. All params already parsed from query string.*/
        case BODYSEC:

            state = PROCESS;
            break;

            /* PROCESS: Dispatch, respond, reset FSM.*/
        case PROCESS:
            wdt_reset();

            if (method == METHOD_GET && uri == URI_DEVICE) {
                send_get_response();
                socket_disconnect(0);

            } else if (method == METHOD_PUT && uri == URI_DEVICE) {
                if (param != PARAM_RESET) {
                    socket_writestr(0, HTTP_400);
                } else if (reset_flag) {
                    socket_writestr(0, HTTP_200);
                    socket_disconnect(0);
                    log_add_record(EVENT_RESET);
                    
                    /*flush to EEPROM*/
                    config_set_modified();
                    config_update_noisr();
                    log_update_noisr();
                    
                    wdt_force_restart();
                    /* no return — wdt_force_restart does not return */
                } else {
                    socket_writestr(0, HTTP_200);
                }

            } else if (method == METHOD_PUT && uri == URI_DEVICE_CONFIG) {
                result = 1;
                if      (param == PARAM_TCRIT_HI) result = update_tcrit_hi(param_value);
                else if (param == PARAM_TWARN_HI) result = update_twarn_hi(param_value);
                else if (param == PARAM_TCRIT_LO) result = update_tcrit_lo(param_value);
                else if (param == PARAM_TWARN_LO) result = update_twarn_lo(param_value);

                socket_writestr(0, (result == 0) ? HTTP_200 : HTTP_400);

            } else if (method == METHOD_DELETE && uri == URI_DEVICE_LOG) {
                log_clear();
                socket_writestr(0, HTTP_200);
                socket_disconnect(0);

            } else {
                socket_writestr(0, HTTP_400);
                socket_disconnect(0);
            }

            socket_disconnect(0);
            http_parserfsm_init();
            break;

        default:
            socket_disconnect(0);
            http_parserfsm_init();
            break;
    }
}
