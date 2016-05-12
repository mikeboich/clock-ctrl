/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
extern int sentence_avail;
extern char sentence[256];

char *get_utc_time();
void init_gps();
// state machine that consumes characters and constructs sentence(s):
void consume_char(char c);

/* [] END OF FILE */
