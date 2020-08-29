#ifndef __OLED_H
#define __OLED_H
#include "Include.h"

void OLED_Config(void);
void OLED_Write_Data(unsigned char data);
void OLED_Set_Address(unsigned char row, unsigned char column);		
void OLED_Full( unsigned char data ); 
void OLED_Write_Str16X8(unsigned char row,unsigned char column, const unsigned char *data );
void OLED_Write_Num16X8( unsigned char row, unsigned char column, int data );

#endif