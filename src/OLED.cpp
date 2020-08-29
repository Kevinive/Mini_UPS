#include "OLED.h"

void OLED_Full( unsigned char data ); 
void OLED_Write_Str16X8(unsigned char row,unsigned char column,const unsigned char *data );


/**********************************************
//IIC Start
**********************************************/
void IIC_Start()
{
   digitalWrite(OLED_SCL,1);
   digitalWrite(OLED_SDA,1);
   digitalWrite(OLED_SDA,0);
   digitalWrite(OLED_SCL,0);
}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop()
{
   digitalWrite(OLED_SCL,0);
   digitalWrite(OLED_SDA,0);
   digitalWrite(OLED_SCL,1);
   digitalWrite(OLED_SDA,1);
}
/**********************************************
// IIC Write byte
**********************************************/
void Write_IIC_Byte(unsigned char IIC_Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)		
	{
		if(IIC_Byte & 0x80)
		digitalWrite(OLED_SDA,1);
		else
		digitalWrite(OLED_SDA,0);
		digitalWrite(OLED_SCL,1);
		digitalWrite(OLED_SCL,0);
		IIC_Byte<<=1;
	}
	digitalWrite(OLED_SDA,1);
	digitalWrite(OLED_SCL,1);
	digitalWrite(OLED_SCL,0);
}

void OLED_Write_Cmd( unsigned char cmd )   //写入命令
{
	IIC_Start();
  Write_IIC_Byte(OLED_ADDR);            //Slave address,SA0=0
  Write_IIC_Byte(0x00);			//write command
  Write_IIC_Byte(cmd); 
  IIC_Stop();
}

void OLED_Write_Data( unsigned char data )    //写入数据
{
	IIC_Start();
  Write_IIC_Byte(OLED_ADDR);			//D/C#=0; R/W#=0
  Write_IIC_Byte(0x40);			//write data
  Write_IIC_Byte(data);
  IIC_Stop();
}

void OLED_Set_Address( unsigned char row, unsigned char column )   //寻址
{
	OLED_Write_Cmd(0xb0+row);
	OLED_Write_Cmd(((column&0xf0)>>4)|0x10);
	OLED_Write_Cmd((column&0x0f)|0x00); 
}

/**************************************************************************************************/
//  给整个液晶的每一位赋值。
//  data=0---->全亮的
//  data=1---->全黑块的  
/**************************************************************************************************/
void OLED_Full( unsigned char data )
{
	unsigned char row,column;
	for( row=0; row<8; row++ )
	{
		OLED_Set_Address(row,0);
		for( column=0; column<128; column++ )
		{
			OLED_Write_Data( data );
		}
	} 
}
void OLED_Config(void)
{
	//I2C总线初始化，此处外部初始化因此注释。
	
	pinMode(OLED_SCL,OUTPUT);
	pinMode(OLED_SDA,OUTPUT);
	
	OLED_Write_Cmd(0xae);//--turn off oled panel
	OLED_Write_Cmd(0x00);//---set low column address
	OLED_Write_Cmd(0x10);//---set high column address
	OLED_Write_Cmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_Write_Cmd(0x81);//--set contrast control register
	OLED_Write_Cmd(0xcf); // Set SEG Output Current Brightness
	OLED_Write_Cmd(0xa1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLED_Write_Cmd(0xc8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLED_Write_Cmd(0xa6);//--set normal display
	OLED_Write_Cmd(0xa8);//--set multiplex ratio(1 to 64)
	OLED_Write_Cmd(0x3f);//--1/64 duty
	OLED_Write_Cmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_Write_Cmd(0x00);//-not offset
	OLED_Write_Cmd(0xd5);//--set display clock divide ratio/oscillator frequency
	OLED_Write_Cmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_Write_Cmd(0xd9);//--set pre-charge period
	OLED_Write_Cmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_Write_Cmd(0xda);//--set com pins hardware configuration
	OLED_Write_Cmd(0x12);
	OLED_Write_Cmd(0xdb);//--set vcomh
	OLED_Write_Cmd(0x40);//Set VCOM Deselect Level
	OLED_Write_Cmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_Write_Cmd(0x02);//
	OLED_Write_Cmd(0x8d);//--set Charge Pump enable/disable
	OLED_Write_Cmd(0x14);//--set(0x10) disable
	OLED_Write_Cmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
	OLED_Write_Cmd(0xa6);// Disable Inverse Display On (0xa6/a7) 
	OLED_Write_Cmd(0xaf);//--turn on oled panel
	OLED_Full( 0 );  //初始清屏
	OLED_Set_Address(0,0);  
}


/***************************************************************************************************/
//    在row行，column列  写一个字符
//    正显的，即字是小黑块的
/***************************************************************************************************/
void OLED_Write_Byte16X8( unsigned char row, unsigned char column, unsigned char data )
{
	unsigned char i;
	OLED_Set_Address( row*2, column*8 );
	for ( i=0; i<8; i++ )
	{
		OLED_Write_Data( pgm_read_byte(&OLED_ASCII16X8[data][i]) );
	}
	OLED_Set_Address( row*2+1, column*8 );
	for ( i=8; i<16; i++ )
	{
		OLED_Write_Data( pgm_read_byte(&OLED_ASCII16X8[data][i]) );
	}
}

/*************************************************************************************************/
//    在row行，column列  写一个字符
//    反显的，即没字的是小黑块的
/*************************************************************************************************/
void OLED_Write_Byte16X8_F( unsigned char row, unsigned char column, unsigned char data )
{
	unsigned char i;
	OLED_Set_Address( row*2, column*8 );
	for ( i=0; i<8; i++ )
	{
		OLED_Write_Data( ~pgm_read_byte(&OLED_ASCII16X8[data][i]) );
	}
	OLED_Set_Address( row*2+1, column*8 );
	for ( i=8; i<16; i++ )
	{
		OLED_Write_Data( ~pgm_read_byte(&OLED_ASCII16X8[data][i]) );
	}
}

/***************************************************************************************************/
//    在row行，column列  写一个字符串
//    正显的，即字的是小黑块的
/***************************************************************************************************/
void OLED_Write_Str16X8(unsigned char row,unsigned char column, const unsigned char *data )
{
	while ( *data != '\0')
	{
		OLED_Write_Byte16X8( row, column, *data );
		column++;
		data++;
	}
}


/***************************************************************************************************/
//    判断data正负，将data转为字符串，高位不足补零   
/***************************************************************************************************/
void OLED_TranstoStr( int data, unsigned char Str[7] )
{
	unsigned char i;
	//判断data正负
	if ( data < 0)
	{
		data = -data;
		Str[5] = '-';
	} 
	else
	{
		Str[5] = '+';
	}
	//将data转为字符串，5位，高位不足补零
	for ( i=5; i>0; i-- )
	{
		Str[i-1] = data%10 + 0x30;
		data /= 10;
	}
	//字符串结束标志
	Str[6]= '\0';
}

/****************************************************************************************************/
//      写入一个int型的数
/****************************************************************************************************/
void OLED_Write_Num16X8( unsigned char row, 
							unsigned char column, int data )
{
	unsigned char buffer[7];
	OLED_TranstoStr( data, buffer );
	OLED_Write_Str16X8( row, column, buffer );
}
/**************************************************************************/


/************************************************************************/
//写小字体  void OLED_Set_num(int pra, unsigned char row, unsigned char pai)
//OLED_Set_num((int)(CarAngle), 4, 1);		pra:变量;row:行;pai:列
//写大字体	OLED_BIG_num((int)(CarSpeed), 0);
/************************************************************************/
void OLED_Write_Byte8X6(unsigned char row, unsigned char column, unsigned char data)
{
	unsigned char i;
	OLED_Set_Address(row, column * 6);
	for (i = 0; i < 8; i++)
	{
		OLED_Write_Data(OLED_ASCII8X6[data][i]);
	}

}
void OLED_Write_Str8X6(unsigned char row, unsigned char column, unsigned char *data)
{
	while (*data != '\0')
	{
		OLED_Write_Byte8X6(row, column, *data);
		column++;
		data++;
	}
}

void OLED_Write_Byte32X32(unsigned char row, unsigned char column, unsigned char data)
{
	unsigned char i;
	OLED_Set_Address(row * 4, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8][i]);
	}
	OLED_Set_Address(row * 4 + 1, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 1][i]);
	}
	OLED_Set_Address(row * 4 + 2, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 2][i]);
	}
	OLED_Set_Address(row * 4 + 3, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 3][i]);
	}

	OLED_Set_Address(row * 4 + 4, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 4][i]);
	}
	OLED_Set_Address(row * 4 + 5, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 5][i]);
	}
	OLED_Set_Address(row * 4 + 6, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 6][i]);
	}
	OLED_Set_Address(row * 4 + 7, column * 32);
	for (i = 0; i < 32; i++)
	{
		OLED_Write_Data(OLED_ASCII32X32[data * 8 + 7][i]);
	}
}

void OLED_Write_Str32X32(unsigned char row, unsigned char *data)
{
	unsigned char column = 0;
	for (column = 0; column < 4; column++)
	{
		OLED_Write_Byte32X32(row, column, *(data + 2));
		data++;
	}

}