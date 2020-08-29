#include "Include.h"

uint8 UPS_rxAdd[] = {0x11,0x22,0x33,0x44,0x55};		//ͨ��1�Ľ��յ�ַ
uint8 Master_txAdd[] = {1,2,3,4,5};
uint8 nrfDataBuf[RXBUFSIZE] = {0};
uint8 displayFlag = 1;					//�˱�����һ֮������˻�������ʼ����0���ʾ�˻���������
uint8 needCalibrate = 1;				//ʱ����ҪУ׼���� ÿ��ϵͳ����������ʾ��ֱ������������ʱ��Ϊֹ��
uint8 systemState = 0;					//ϵͳ״̬bit���������¹�����
uint16 chargeStartTime = 0;			//��¼��翪ʼ��ʱ�� (min)
uint16 sysCount = 0;

//�����ѹ��׼������Ҫ����
uint16 adapter_refVoltage = REF_VOLTAGE;
uint16 battery_fullVoltage = REF_BATFULL_VOLTAGE;
uint16 battery_emptyVoltage = REF_BATEMPTY_VOLTAGE;
uint16 battery_needChargeVoltage = REF_BATNEEDCHG_VOLTAGE;
uint16 maxChargeTime = CHARGER_TIME;

/*
Bit		7		6		5		4		3		2		1		Func
									ICH	CHG	PW1	PW0
ICH(is Charge Needed)��	�Ƿ����ڳ�磿				0��1��
CHG(Charge)��						����Ƿ���Ҫ��磿		0��1��
PW1&PW0(PowerState):		ϵͳ��Դ��						00�е�	01���	10�ϵ磺��ر���

*/

uint8 wakeCount = 0;


RTCTime currentTime = {0,48,20,25,9,18,0};
RTCTime AlarmTime[2] = {{0,55,22,0,0,0,0},{0,0,0,0,0,0,0}};			//��ʼ��ֵΪ22��55�ֵ�����һ��������ṹ��ȫΪ0���������ӡ�

bool h12,hPM;
DS3231 Clock;				//��������

VoltInfo volInfo[3] = {6,"Adapter",0,
											 7,"Charger",0,
											 3,"Battery",0};

uint16 U_Adapter, U_Charger, U_Battery;

void setup(){
	NRF24L01_InitTypeDef NRF24L01_InitStructure;
	
	pinMode(RELAY1_PIN,OUTPUT);
	pinMode(RELAY2_PIN,OUTPUT);
	pinMode(RELAY3_PIN,OUTPUT);
	
	//digitalWrite(RELAY3_PIN,1);				//��Ϊ����ԭ��˹�����ͣ
	switchToAdapter();
	turnOffCharger();
	
	//NRFģ���ʼ��������SPI��ʼ����
  NRF24L01_init();
  NRF24L01_InitStructure.IR_MASK = NRF24L01_MASK_RX_DR|NRF24L01_MASK_TX_DS|NRF24L01_MASK_MAX_RT;
  NRF24L01_InitStructure.CRC_CONFIG = NRF24L01_CRC_EN|NRF24L01_CRC_8bit;
  NRF24L01_InitStructure.EN_AA = NRF24L01_ENAA_ALL;
  NRF24L01_InitStructure.EN_RXADDR = NRF24L01_ERX_P0|NRF24L01_ERX_P1;
  NRF24L01_InitStructure.AW = NRF24L01_AW_5b;
  NRF24L01_InitStructure.RETR_ARD = NRF24L01_RETR_ARD_1000us;
  NRF24L01_InitStructure.RETR_ARC = 3;
  NRF24L01_InitStructure.RF_CH = NRF24L01_RF_CH_DEFAULT;
  NRF24L01_InitStructure.RF_DR = NRF24L01_RF_DR_2M;
  NRF24L01_InitStructure.RF_PWR = NRF24L01_RF_PWR_0;
  NRF24L01_InitStructure.RX_ADDR[0] = Master_txAdd;
  NRF24L01_InitStructure.RX_ADDR[1] = UPS_rxAdd;
  NRF24L01_InitStructure.RX_PW[0] = RXBUFSIZE;
  NRF24L01_InitStructure.RX_PW[1] = RXBUFSIZE;
  NRF24L01_config(&NRF24L01_InitStructure);
	
	Serial.begin(115200);
	//I2C��ʼ��
  Wire.begin();
	OLED_Config();
	
	OLED_Write_Str16X8(0,0,"SOURCE:");
	OLED_Write_Str16X8(1,0,"Adapter:");
	OLED_Write_Str16X8(2,0,"Charger:");
	OLED_Write_Str16X8(3,0,"Battery:");
	
	//RTC��ʼ����������I2C��ʼ����
  //RTCһֱ�������е��У������ʼ��������Ҫ�����ý�Ϊʱ����趨��
  //�˴�����RTC��ʱ������
	/*Clock.setSecond(currentTime.second);//Set the second 
	Clock.setMinute(currentTime.minute);//Set the minute 
	Clock.setHour(currentTime.hour);  //Set the hour 
	Clock.setDoW(currentTime.DoW);    //Set the day of the week
	Clock.setDate(currentTime.date);  //Set the date of the month
	Clock.setMonth(currentTime.month);  //Set the month of the year
	Clock.setYear(currentTime.year);  //Set the year (Last two digits of the year)
	*/
	
	currentTime.second = Clock.getSecond();
	currentTime.minute = Clock.getMinute();
  currentTime.hour = Clock.getHour(h12, hPM);
  currentTime.date = Clock.getDate();
  currentTime.month = Clock.getMonth(h12);
  currentTime.year = Clock.getYear();
  
  //�˴�����RTC����������
  Clock.setA1Time(AlarmTime[0].date, AlarmTime[0].hour, AlarmTime[0].minute, AlarmTime[0].second, ALM1BIT_HMS, false, false, false); 
  Clock.setA2Time(AlarmTime[1].date, AlarmTime[1].hour, AlarmTime[1].minute, ALM2BIT_HMS, false, false, false);
  if(AlarmTime[0].second | AlarmTime[0].hour | AlarmTime[0].minute | AlarmTime[0].date | AlarmTime[0].month | AlarmTime[0].year)
  	Clock.turnOnAlarm(1);
  else
  	Clock.turnOffAlarm(1);
  	
  if(AlarmTime[1].second | AlarmTime[1].hour | AlarmTime[1].minute | AlarmTime[1].date | AlarmTime[1].month | AlarmTime[1].year)
  	Clock.turnOnAlarm(2);
  else
  	Clock.turnOffAlarm(2);
  
  if(EEPROM.read(0) == 0xA8){
  	adapter_refVoltage = EEPROM.read(1)<<8;
  	adapter_refVoltage |= EEPROM.read(2);
		battery_fullVoltage = EEPROM.read(3)<<8;
		battery_fullVoltage |= EEPROM.read(4);
		battery_emptyVoltage = EEPROM.read(5)<<8;
		battery_emptyVoltage |= EEPROM.read(6);
		maxChargeTime = EEPROM.read(7)<<8;
		maxChargeTime |= EEPROM.read(8);
		battery_needChargeVoltage = EEPROM.read(9)<<8;
		battery_needChargeVoltage |= EEPROM.read(10);
  }else{
  	EEPROM.write(0, 0xA8);
  	EEPROM.write(1, (char)(adapter_refVoltage>>8));
  	EEPROM.write(2, (char)(adapter_refVoltage&0xff));
  	EEPROM.write(3, (char)(battery_fullVoltage>>8));
  	EEPROM.write(4, (char)(battery_fullVoltage&0xff));
  	EEPROM.write(5, (char)(battery_emptyVoltage>>8));
  	EEPROM.write(6, (char)(battery_emptyVoltage&0xff));
  	EEPROM.write(7, (char)(maxChargeTime>>8));
  	EEPROM.write(8, (char)(maxChargeTime&0xff));
  	EEPROM.write(9, (char)(battery_needChargeVoltage>>8));
  	EEPROM.write(10, (char)(battery_needChargeVoltage&0xff));
  }
  
	WDTwake_init();
}

void loop(){
	//�˴��Ųɼ�����
	getVoltages();
	if(wakeCount >= 8){			//���Ѽ��8�룬�˴����Ѷ�ȡһ��ʱ�䣬����һ����
		currentTime.second = Clock.getSecond();
		currentTime.minute = Clock.getMinute();
  	currentTime.hour = Clock.getHour(h12, hPM);
  	currentTime.date = Clock.getDate();
  	currentTime.month = Clock.getMonth(h12);
  	currentTime.year = Clock.getYear();
  	wakeCount = 0;
	}
	batteryManager();
	wakeCount++;
	sysCount++;
	OLED_Write_Num16X8(0,0,sysCount);
	OLED_Write_Num16X8(0,10,systemState);
  OLED_Write_Num16X8(1,10,volInfo[0].value);
  OLED_Write_Num16X8(2,10,volInfo[1].value);
  OLED_Write_Num16X8(3,10,volInfo[2].value);
	
	//�˴����������˻������������ش���
  /*if(displayFlag){
  	
  	
  	attachInterrupt(digitalPinToInterrupt(2),UPS_Int0_Handler,CHANGE);			//����ж�ʹ��
  	displayFlag = 0;
  }
  */
  
  //��������ݴ���
  //NRF24L01_PrintState();
	if(NRF24L01_getStatus() & NRF24L01_STATU_RX_DR){
		NRF24L01_recieve(nrfDataBuf);			//��������
		
		if(nrfDataBuf[0] == UPS_rxAdd[4] && nrfDataBuf[1] == UPS_rxAdd[3] && nrfDataBuf[2] == UPS_rxAdd[2] && nrfDataBuf[3] == UPS_rxAdd[1] && nrfDataBuf[4] == UPS_rxAdd[0]){
			//��ֵ
			adapter_refVoltage = ((uint16)nrfDataBuf[5])<<8 | nrfDataBuf[6];
			battery_fullVoltage = ((uint16)nrfDataBuf[7])<<8 | nrfDataBuf[8];
			battery_emptyVoltage = ((uint16)nrfDataBuf[9])<<8 | nrfDataBuf[10];
			maxChargeTime = ((uint16)nrfDataBuf[11])<<8 | nrfDataBuf[12];
			battery_needChargeVoltage = ((uint16)nrfDataBuf[13])<<8 | nrfDataBuf[14];
			
			//�洢
			EEPROM.write(1, (char)(adapter_refVoltage>>8));
	  	EEPROM.write(2, (char)(adapter_refVoltage&0xff));
	  	EEPROM.write(3, (char)(battery_fullVoltage>>8));
	  	EEPROM.write(4, (char)(battery_fullVoltage&0xff));
	  	EEPROM.write(5, (char)(battery_emptyVoltage>>8));
	  	EEPROM.write(6, (char)(battery_emptyVoltage&0xff));
	  	EEPROM.write(7, (char)(maxChargeTime>>8));
	  	EEPROM.write(8, (char)(maxChargeTime&0xff));
	  	EEPROM.write(9, (char)(battery_needChargeVoltage>>8));
  		EEPROM.write(10, (char)(battery_needChargeVoltage&0xff));
			
			//����Ϊ���Գ���
			OLED_Write_Num16X8(0,10,maxChargeTime);
			OLED_Write_Num16X8(1,10,adapter_refVoltage);
  		OLED_Write_Num16X8(2,10,battery_fullVoltage);
  		OLED_Write_Num16X8(3,10,battery_emptyVoltage);
			OLED_Write_Str16X8(0,10,"OK");
			OLED_Write_Str16X8(1,10,"OK");
			OLED_Write_Str16X8(2,10,"OK");
			OLED_Write_Str16X8(3,10,"OK");
		}
		//��ջ�����
		memset(nrfDataBuf, 0, sizeof(nrfDataBuf));
	}
	
	
	
  Serial.print(currentTime.year);
  Serial.print(".");
  Serial.print(currentTime.month);
  Serial.print(".");
  Serial.println(currentTime.date);
  
  Serial.print("time:\t");
  Serial.print(currentTime.hour);
  Serial.print(":");
  Serial.print(currentTime.minute);
  Serial.print(":");
  Serial.println(currentTime.second);
  delay(50);
  
  
  /*if((systemState & UPS_STATE_PW1_MSK == 0) && (systemState & UPS_STATE_PW0_MSK == 0)){
  	OLED_Write_Str16X8(0,10,"Adap");
  }else if((systemState & UPS_STATE_PW1_MSK == 0) && (systemState & UPS_STATE_PW0_MSK)){
  	OLED_Write_Str16X8(0,10,"Batt");
  }*/
  
	WDTwake_enterSleep();
}

void getVoltages(){
	uint8 i;
	for(i=0;i<3;i++){
		volInfo[i].value = analogRead(volInfo[i].pin);
	}
	U_Adapter = volInfo[0].value;
	U_Charger = volInfo[1].value;
	U_Battery = volInfo[2].value;
	
	return;
}


//��Դ�����߼�
/*
Bit		7		6		5		4		3		2		1		Func
									ICH	CHG	PW1	PW0
ICH(is Charge Needed)��	�Ƿ����ڳ�磿				0��1��
CHG(Charge)��						����Ƿ���Ҫ��磿		0��1��
PW1&PW0(PowerState):		ϵͳ��Դ��						00�е�	01���	10�ϵ磺��ر���

*/
void batteryManager(){
	switch(systemState & (UPS_STATE_PW1_MSK | UPS_STATE_PW0_MSK)){
		case 0:				//��ǰ����Ϊ�е�
			if(U_Adapter > adapter_refVoltage){		//��Դ�������е�
				if(((uint16)currentTime.hour*60 + currentTime.minute) > ((uint16)AlarmTime[0].hour*60 + AlarmTime[0].minute) && (systemState & UPS_STATE_CHG_MSK) == 0){
					switchToBat();
				}
				if(U_Battery < battery_needChargeVoltage){
					turnOnCharger();
					chargeStartTime = ((uint16)currentTime.hour*60 + currentTime.minute);
				}
				if((UPS_STATE_ICH_MSK & systemState) && (((uint16)currentTime.hour*60 + currentTime.minute) > (chargeStartTime + maxChargeTime))){
					turnOffCharger();
					systemState &= ~UPS_STATE_CHG_MSK;
				}
			}else if((systemState & UPS_STATE_CHG_MSK) == 0){		//��Դ������û���ҵ���е�
				switchToBat();
			}else{				//��Դ������û���ҵ��û��
				
			}
			break;
		case 1:				//��ǰ����Ϊ��ع���
			if(U_Adapter > adapter_refVoltage && ((uint16)currentTime.hour*60 + currentTime.minute) < ((uint16)AlarmTime[0].hour*60 + AlarmTime[0].minute)){
				switchToAdapter();
				//turnOnCharger();			//ÿ�δӵ���л��������Զ������������磨�������Ե�ز��ã�
				//chargeStartTime = ((uint16)currentTime.hour*60 + currentTime.minute);
			}else if(U_Battery < battery_emptyVoltage){
				switchToAdapter();
				systemState |= 0x06;				//�л����ϵ�ģʽ
				systemState &= ~0x01;
			}else{
				
			}
			break;
		case 2:				//��ǰΪ�ϵ�������ر���
			if(U_Charger > adapter_refVoltage){
				turnOnCharger();
				chargeStartTime = currentTime.hour;
				systemState &= ~0x07;
			}
			break;
		default:
			break;
	}
}

void switchToBat(){
	digitalWrite(RELAY1_PIN,1);
	systemState |= 0x01;
	systemState &= ~0x02;
}

void switchToAdapter(){
	digitalWrite(RELAY1_PIN,0);
	systemState &= ~0x03;
}

void turnOnCharger(){
	digitalWrite(RELAY2_PIN,1);
	systemState |= 0x08;
}

void turnOffCharger(){
	digitalWrite(RELAY2_PIN,0);
	systemState &= ~0x08;
}
