#ifndef IO_H_
#define IO_H_


void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteData(unsigned char Data);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const unsigned char *string);
void delay_ms(int miliSec);

unsigned char * LCD_To_String(unsigned short num, unsigned char * string, unsigned char size);


#endif /* IO_H_ */

