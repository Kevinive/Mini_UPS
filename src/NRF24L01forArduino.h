#ifndef _NRF24L01forArduino_H
#define _NRF24L01forArduino_H
#include "SPI.h"

#define uint_8 unsigned char
/*
	
*/
extern uint_8 nrf24L01_CR,nrf24L01_Aw;

#define NRF24L01_INT_Pin			3
#define NRF24L01_CSN_Pin			10
#define NRF24L01_CE_Pin				9
//#define NRF24L01_CSN_BK_Pin		6
//#define NRF24L01_CE_BK_Pin		7
#define NRF24L01_SCK_Pin			13
#define NRF24L01_MOSI_Pin			11
#define NRF24L01_MISO_Pin			12

#define NRF24L01_CSN_H			(digitalWrite(NRF24L01_CSN_Pin,1))
#define NRF24L01_CSN_L			(digitalWrite(NRF24L01_CSN_Pin,0))
#define NRF24L01_CE_H				({digitalWrite(NRF24L01_CE_Pin,1);delayMicroseconds(130);})
#define NRF24L01_CE_L				(digitalWrite(NRF24L01_CE_Pin,0))
#define NRF24L01_CSN_BK_H			(digitalWrite(NRF24L01_CSN_BK_Pin,1))
#define NRF24L01_CSN_BK_L			(digitalWrite(NRF24L01_CSN_BK_Pin,0))
#define NRF24L01_CE_BK_H				(digitalWrite(NRF24L01_CE_BK_Pin,1))
#define NRF24L01_CE_BK_L				(digitalWrite(NRF24L01_CE_BK_Pin,0))

#define NRF24L01_PRIM_RX								0X01
#define NRF24L01_PRIM_TX								0X00

//寄存器操作命令
#define NRF24L01_R_REG								0X00
#define NRF24L01_W_REG								0X20
#define NRF24L01_R_RX_PAYLOAD					0X61
#define NRF24L01_W_TX_PAYLOAD					0XA0
#define NRF24L01_FLUSH_TX							0XE1
#define NRF24L01_FLUSH_RX							0XE2
#define NRF24L01_REUSE_TX_PAYLOAD			0XE3
#define NRF24L01_NOP									0XFF

//寄存器地址
#define NRF24L01_CONFIG								0X00
#define NRF24L01_EN_AA								0X01
#define NRF24L01_EX_RXADDR						0X02
#define NRF24L01_SETUP_AW							0X03
#define NRF24L01_SETUP_RATE						0X04
#define NRF24L01_RF_CH								0X05
#define NRF24L01_RF_SETUP							0X06
#define NRF24L01_STATUS								0X07
#define NRF24L01_OBSERVE_TX						0X08
#define NRF24L01_CD										0X09
#define NRF24L01_RX_ADDR_P0						0X0A
#define NRF24L01_RX_ADDR_P1						0X0B
#define NRF24L01_RX_ADDR_P2						0X0C
#define NRF24L01_RX_ADDR_P3						0X0D
#define NRF24L01_RX_ADDR_P4						0X0E
#define NRF24L01_RX_ADDR_P5						0X0F
#define NRF24L01_TX_ADDR							0X10
#define NRF24L01_RX_PW_P0							0X11
#define NRF24L01_RX_PW_P1							0X12
#define NRF24L01_RX_PW_P2							0X13
#define NRF24L01_RX_PW_P3							0X14
#define NRF24L01_RX_PW_P4							0X15
#define NRF24L01_RX_PW_P5							0X16
#define NRF24L01_FIFO_STATUS					0X17



//---------------------------------------------------------------------------------------------------
//初始化相关
typedef struct NRF24L01InitTypeDef
{
	uint_8 IR_MASK;			//NRF24L01_MASK_RX_DR;NRF24L01_MASK_TX_DS;NRF24L01_MASK_MAX_RT
	uint_8 CRC_CONFIG;		//NRF24L01_CRC_EN;NRF24L01_CRC_8bit;NRF24L01_CRC_16bit
	uint_8 EN_AA;			//NRF24L01_ENAA_ALL;NRF24L01_ENAA_Px
	uint_8 EN_RXADDR;		//NRF24L01_ERX_ALL;NRF24L01_ERX_Px
	uint_8 AW;				//NRF24L01_AW_3b;4b;5b
	uint_8 RETR_ARD;		//NRF24L01_RETR_ARD_xxxus
	uint_8 RETR_ARC;		//uint_8 and <16
	uint_8 RF_CH;				//NRF24L01_RF_CH_DEFAULT
	uint_8 RF_DR;				//NRF24L01_RF_DR_1M/2M
	uint_8 RF_PWR;			//NRF24L01_RF_PWR_0/_6/_12/_18
	uint_8* RX_ADDR[6];			//pointer array
	uint_8 RX_PW[6];				//设置五个接收信道有效数据宽度
}NRF24L01_InitTypeDef;


#define NRF24L01_MASK_RX_DR				0X40
#define NRF24L01_MASK_TX_DS				0X20
#define NRF24L01_MASK_MAX_RT			0X10

#define NRF24L01_CRC_EN						0X08
#define NRF24L01_CRC_8bit					0x00
#define NRF24L01_CRC_16bit				0x04

#define NRF24L01_ENAA_ALL				0x3F
#define NRF24L01_ENAA_P0				0x01
#define NRF24L01_ENAA_P1				0x02
#define NRF24L01_ENAA_P2				0x04
#define NRF24L01_ENAA_P3				0x08
#define NRF24L01_ENAA_P4				0x10
#define NRF24L01_ENAA_P5				0x20

#define NRF24L01_ERX_ALL				0x3F
#define NRF24L01_ERX_P0					0x01
#define NRF24L01_ERX_P1					0x02
#define NRF24L01_ERX_P2					0x04
#define NRF24L01_ERX_P3					0x08
#define NRF24L01_ERX_P4					0x10
#define NRF24L01_ERX_P5					0x20

#define NRF24L01_AW_3b					0x01
#define NRF24L01_AW_4b					0x02
#define NRF24L01_AW_5b					0x03

#define NRF24L01_RETR_ARD_250us			0x00
#define NRF24L01_RETR_ARD_500us			0x10
#define NRF24L01_RETR_ARD_750us			0x20
#define NRF24L01_RETR_ARD_1000us		0x30
#define NRF24L01_RETR_ARD_1250us		0x40
#define NRF24L01_RETR_ARD_1500us		0x50
#define NRF24L01_RETR_ARD_1750us		0x60
#define NRF24L01_RETR_ARD_2000us		0x70
#define NRF24L01_RETR_ARD_2250us		0x80
#define NRF24L01_RETR_ARD_2500us		0x90
#define NRF24L01_RETR_ARD_2750us		0xA0
#define NRF24L01_RETR_ARD_3000us		0xB0
#define NRF24L01_RETR_ARD_3250us		0xC0
#define NRF24L01_RETR_ARD_3500us		0xD0
#define NRF24L01_RETR_ARD_3750us		0xE0
#define NRF24L01_RETR_ARD_4000us		0xF0

#define NRF24L01_RF_CH_DEFAULT			0x02

#define NRF24L01_RF_DR_1M						0x00
#define NRF24L01_RF_DR_2M						0x08

#define NRF24L01_RF_PWR__18					0x01
#define NRF24L01_RF_PWR__12					0x03
#define NRF24L01_RF_PWR__6					0x05
#define NRF24L01_RF_PWR_0						0x07
//------------------------------------------------------------------------------------------------

//状态寄存器状态判断
#define NRF24L01_STATU_RX_DR					0x40
#define NRF24L01_STATU_TX_DS					0x20
#define NRF24L01_STATU_MAX_RT					0x10
#define NRF24L01_STATU_RX_P_NO				0x0E
#define NRF24L01_STATU_TX_FULL				0X01

#define NRF24L01_FIFO_STATU_TX_REUSE					0x40
#define NRF24L01_FIFO_STATU_TX_FULL						0x20
#define NRF24L01_FIFO_STATU_TX_EMPTY					0x10
#define NRF24L01_FIFO_STATU_RX_FULL						0x02
#define NRF24L01_FIFO_STATU_RX_EMPTY					0x01


//-------------------------------------------------------------------------------------------------

void NRF24L01_init(void);
void NRF24L01_config(NRF24L01_InitTypeDef* stru);
uint_8 NRF24L01_send(const uint_8* txAdd, const uint_8* datBuff, uint_8 byteNum);
uint_8 NRF24L01_recieve(uint_8* readBuf);
void NRF24L01_PrintState(void);
void NRF24L01_setPwr(uint_8 state);
void NRF24L01_setPrim(uint_8 state);
void NRF24L01_setTxAdd(const uint_8* add);
void NRF24L01_setRxAdd(uint_8 channel, const uint_8* add);
void NRF24L01_getRxAdd(uint_8 channel, uint_8* add);
uint_8 NRF24L01_getStatus(void);
uint_8 NRF24L01_getFifoState(void);
void NRF24L01_setStatus(uint_8 stat);
void NRF24L01_loadPayload(const uint_8* payload, uint_8 size);
void NRF24L01_readPayload(uint_8* payload, uint_8 size);
void NRF24L01_flushTx(void);
void NRF24L01_flushRx(void);
void NRF24L01_WriteOneByte(uint_8 cmd, uint_8 dat);
void NRF24L01_WriteBytes(uint_8 cmd, const uint_8* datas, uint_8 byteNum);
uint_8 NRF24L01_ReadOneByte(uint_8 cmd);
void NRF24L01_ReadBytes(uint_8 cmd, uint_8* datas, uint_8 byteNum);
static void NRF24L01_WriteCmd(uint_8 cmd);
static uint_8 NRF24L01_SendData(uint_8 dat);


#endif
