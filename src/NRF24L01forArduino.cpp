#include "NRF24L01forArduino.h"


uint_8 nrf24L01_CR = 0X08, nrf24L01_Aw = 5;

void NRF24L01_init(){
	
	pinMode(NRF24L01_CSN_Pin,OUTPUT);
  pinMode(NRF24L01_CE_Pin,OUTPUT);
  //pinMode(NRF24L01_CSN_BK_Pin,OUTPUT);
  //pinMode(NRF24L01_CE_BK_Pin,OUTPUT);
  pinMode(NRF24L01_SCK_Pin,OUTPUT);
  pinMode(NRF24L01_MOSI_Pin,OUTPUT);
  pinMode(NRF24L01_MISO_Pin,INPUT);
	
	SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
	
	NRF24L01_CE_L;
	NRF24L01_CSN_H;
}


//NRF24L01 初始参数设定
void NRF24L01_config(NRF24L01_InitTypeDef* stru){
	uint_8 i,aw;
	aw = (*stru).AW + 2;
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_CONFIG, 			(*stru).IR_MASK|(*stru).CRC_CONFIG);
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_EN_AA, 				(*stru).EN_AA);
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_EX_RXADDR, 		(*stru).EN_RXADDR);
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_SETUP_AW, 		(*stru).AW);
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_SETUP_RATE, 	(*stru).RETR_ARD|(*stru).RETR_ARC);
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_RF_CH, 				(*stru).RF_CH);
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_RF_SETUP, 		(*stru).RF_DR|(*stru).RF_PWR);
	
	for(i=0;i<6;i++){
		if(((*stru).EN_RXADDR) & (1<<i)){
			NRF24L01_WriteBytes(NRF24L01_W_REG|(NRF24L01_RX_ADDR_P0+i), *((*stru).RX_ADDR+i), (i<2?aw:1));			//!!!!!
			NRF24L01_WriteOneByte(NRF24L01_W_REG|(NRF24L01_RX_PW_P0+i), (*stru).RX_PW[(i<=32?i:32)]);
		}
	}
	Serial.print("NRF24L01 Config Complete...\r\n");
	
	nrf24L01_CR = (*stru).IR_MASK|(*stru).CRC_CONFIG;
	nrf24L01_Aw = aw;
	NRF24L01_setPrim(NRF24L01_PRIM_RX);
	NRF24L01_setPwr(1);
	NRF24L01_CE_H;
}


//收发
uint_8 NRF24L01_send(const uint_8* txAdd, const uint_8* datBuff, uint_8 byteNum){
	uint_8 tmpRxAdd[5],stateByte, ret;
	Serial.print("Loading payload......");
	NRF24L01_CE_L;
	
	//切换为发送模式
	NRF24L01_setPrim(NRF24L01_PRIM_TX);
	if(byteNum >32) byteNum = 32;
	
	//备份0通道地址
	NRF24L01_setTxAdd(txAdd);
	NRF24L01_getRxAdd(0, tmpRxAdd);
	NRF24L01_setRxAdd(0, txAdd);
	
	//清除中断标志位
	stateByte = NRF24L01_getStatus();
	if(stateByte & NRF24L01_STATU_TX_DS) NRF24L01_setStatus(NRF24L01_STATU_TX_DS);
	if(stateByte & NRF24L01_STATU_MAX_RT) NRF24L01_setStatus(NRF24L01_STATU_MAX_RT);
	
	//清空后写入数据
	NRF24L01_flushTx();
	NRF24L01_loadPayload(datBuff, byteNum);
	
	//发送
	NRF24L01_CE_H;
	
	Serial.print("power on!\r\n");
	//判断发送是否成功
	do{
		stateByte = NRF24L01_getStatus();
		if(stateByte & NRF24L01_STATU_TX_DS){
			NRF24L01_setStatus(NRF24L01_STATU_TX_DS);
			ret = 1;
			break;
		}
		if(stateByte & NRF24L01_STATU_MAX_RT){
			NRF24L01_setStatus(NRF24L01_STATU_MAX_RT);
			ret = 0;
			break;
		}
		Serial.print("stateByte:");
		Serial.println(stateByte,HEX);
		delay(500);
	}while(1);
	//结束发送
	NRF24L01_CE_L;
	//还原0通道地址
	NRF24L01_setRxAdd(0, tmpRxAdd);
	
	//切换为RX模式
	NRF24L01_setPrim(NRF24L01_PRIM_RX);
	NRF24L01_CE_H;
	return ret;
}

uint_8 NRF24L01_recieve(uint_8* readBuf){
	uint_8 ch, len, stateByte;
	NRF24L01_CE_L;
	ch = (NRF24L01_getStatus() & NRF24L01_STATU_RX_P_NO) >> 1;
	
	stateByte = NRF24L01_getStatus();
	if(stateByte & NRF24L01_STATU_RX_DR) NRF24L01_setStatus(NRF24L01_STATU_RX_DR);
	if(ch == 7){
		Serial.print("No data in RX FIFO Buffer.....\r\n");
		return 7;
	}
	
	len = NRF24L01_ReadOneByte(NRF24L01_R_REG|(NRF24L01_RX_PW_P0+ch));
	
	NRF24L01_readPayload(readBuf, len);
	Serial.print("Get data from ch:");
	Serial.print(ch,HEX);
	Serial.print("\tlen:");
	Serial.println(len,HEX);
	NRF24L01_flushRx();			//！！！
	NRF24L01_CE_H;
	return ch;
}


void NRF24L01_PrintState(){
	Serial.print("NRF_CONFIG:");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_CONFIG));
  Serial.print("NRF_ENAA");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_EN_AA));
  Serial.print("NRF_RXADDR");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_EX_RXADDR));
  Serial.print("NRF_AW");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_SETUP_AW));
  Serial.print("NRF_SETUPRATE");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_SETUP_RATE));
  Serial.print("NRF_RFCH");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_RF_CH));
  Serial.print("NRF_RFSETUP");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_RF_SETUP));
  Serial.print("NRF_STATUS");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_STATUS));
  Serial.print("NRF_ADDR1");
  Serial.println(NRF24L01_ReadOneByte(NRF24L01_RX_ADDR_P1));
  
}


//基本操作
void NRF24L01_setPwr(uint_8 state){
	if(state) nrf24L01_CR |= 0x02;
	else nrf24L01_CR &= 0xFD;
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_CONFIG, nrf24L01_CR);
	delay(2);
}

void NRF24L01_setPrim(uint_8 state){				//1收0发
	if(state) nrf24L01_CR |= 0x01;
	else nrf24L01_CR &= 0xFE;
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_CONFIG, nrf24L01_CR);
}

void NRF24L01_setTxAdd(const uint_8* add){
	NRF24L01_WriteBytes(NRF24L01_W_REG|NRF24L01_TX_ADDR, add, nrf24L01_Aw);
}

void NRF24L01_setRxAdd(uint_8 channel, const uint_8* add){
	NRF24L01_WriteBytes(NRF24L01_W_REG|(NRF24L01_RX_ADDR_P0+channel), add, nrf24L01_Aw);
}

void NRF24L01_getRxAdd(uint_8 channel, uint_8* add){
	NRF24L01_ReadBytes(NRF24L01_R_REG|(NRF24L01_RX_ADDR_P0+channel), add, nrf24L01_Aw);
}

uint_8 NRF24L01_getStatus(){
	return NRF24L01_ReadOneByte(NRF24L01_R_REG|NRF24L01_STATUS);
}

uint_8 NRF24L01_getFifoState(){
	return NRF24L01_ReadOneByte(NRF24L01_R_REG|NRF24L01_FIFO_STATUS);
}

void NRF24L01_setStatus(uint_8 stat){
	NRF24L01_WriteOneByte(NRF24L01_W_REG|NRF24L01_STATUS, stat);
}

void NRF24L01_loadPayload(const uint_8* payload, uint_8 size){
	NRF24L01_WriteBytes(NRF24L01_W_TX_PAYLOAD , payload, size);
}

void NRF24L01_readPayload(uint_8* payload, uint_8 size){
	NRF24L01_ReadBytes(NRF24L01_R_RX_PAYLOAD , payload, size);
}

void NRF24L01_flushTx(){
	NRF24L01_CSN_L;
	NRF24L01_WriteCmd(NRF24L01_FLUSH_TX);
	NRF24L01_CSN_H;
}

void NRF24L01_flushRx(){
	NRF24L01_CSN_L;
	NRF24L01_WriteCmd(NRF24L01_FLUSH_RX);
	NRF24L01_CSN_H;
}


//基础操作------------------------------------------------------------------------------------------------
void NRF24L01_WriteOneByte(uint_8 cmd, uint_8 dat){
	NRF24L01_CE_L;
	NRF24L01_CSN_L;
	NRF24L01_WriteCmd(cmd);
	NRF24L01_SendData(dat);
	NRF24L01_CSN_H;
}

void NRF24L01_WriteBytes(uint_8 cmd, const uint_8* datas, uint_8 byteNum){
	uint_8 i;
	NRF24L01_CE_L;
	NRF24L01_CSN_L;
	NRF24L01_WriteCmd(cmd);
	datas += (byteNum-1);
	for(i=0;i<byteNum;i++){
		NRF24L01_SendData(*datas);
		datas--;
	}
	NRF24L01_CSN_H;
}

uint_8 NRF24L01_ReadOneByte(uint_8 cmd){
	uint_8 dat;
	NRF24L01_CSN_L;
	NRF24L01_WriteCmd(cmd);
	dat = NRF24L01_SendData(0xff);
	NRF24L01_CSN_H;
	return dat;
}

void NRF24L01_ReadBytes(uint_8 cmd, uint_8* datas, uint_8 byteNum){
	uint_8 i;
	NRF24L01_CSN_L;
	NRF24L01_WriteCmd(cmd);
	datas += (byteNum-1);
	for(i=0;i<byteNum;i++){
		*datas = NRF24L01_SendData(0xff);
		datas--;
	}
	NRF24L01_CSN_H;
}


static void NRF24L01_WriteCmd(uint_8 cmd){
	NRF24L01_SendData(cmd);
}

static uint_8 NRF24L01_SendData(uint_8 dat){
	return SPI.transfer(dat);
}
