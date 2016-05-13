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

void init_gps();
void send_command(char *s); // send a command with checksum and crlf

// state machine that consumes characters and constructs sentence(s):
void consume_char(char c);

/* [] END OF FILE */
