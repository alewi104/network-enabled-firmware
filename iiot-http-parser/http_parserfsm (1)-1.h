/********************************************************
 * http_parserfsm.h
 *
 * Provides function declarations for the HTTP parser
 * finite state machine.
 *
 * Author:   Ahlaireah Lewis
 * Date:     4/30/2026
 * Revision: 1.0
 *
 ********************************************************/
#ifndef	_HTTP_PARSERFSM_H_
#define	_HTTP_PARSERFSM_H_


/* Initialize HTTP parser FSM state */
void http_parserfsm_init();

/* Execute one FSM update step */
void http_parserfsm_update();

#endif
/*_HTTP_PARSERFSM_H_ */
