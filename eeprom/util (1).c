/********************************************************
 * util.c
 *
 * Description:
 * Provides utility functions for checksum generation and validation.
 *
 * Author:  Ahlaireah Lewis
 * Course:  SER486
 * Assignment: Project 2
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/

/********************************************************
 * Function: update_checksum
 *
 * Description:
 * Computes and stores checksum as two's complement of sum
 * of all bytes except the last.
 *
 * Parameters:
 * data - pointer to data buffer
 * size - total size of buffer
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * None
 *
 * Notes:
 * - Last byte is reserved for checksum
 ********************************************************/
void update_checksum(unsigned char* data, unsigned int size) {
    unsigned char i;
    unsigned char sum = 0;
    for (i = 0; i < size-1; i++){
        sum+= data[i];
    }
    data[size-1] = 0 - sum;
 }

/********************************************************
 * Function: is_checksum_valid
 *
 * Description:
 * Validates checksum by recomputing sum and verifying result.
 *
 * Parameters:
 * data - pointer to data buffer
 * size - total size of buffer
 *
 * Returns:
 * int
 *  - 1: valid
 *  - 0: invalid
 *
 * Hardware Effects:
 * None
 ********************************************************/ 
int is_checksum_valid(unsigned char* data, unsigned int size){
    unsigned char i;
    unsigned char sum = 0;
    for (i = 0; i < size-1; i++){
        sum+= data[i];
    }

    return (data[size-1]- sum == 0); 
 }
