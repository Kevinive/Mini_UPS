#include "Include.h"

uint8 UPS_rxAdd[] = {0x11,0x22,0x33,0x44,0x55};		//通道1的接收地址
uint8 Master_txAdd[] = {1,2,3,4,5};
uint8 nrfDataBuf[RXBUFSIZE] = {0};
uint8 displayFlag = 1;					//此变量置一之后表明人机互动开始，置0则表示人机互动结束
uint8 needCalibrate = 1;				//时间需要校准变量 每次系统重启都会提示，直到重新设置了时间为止。
uint8 systemState = 0;					//系统状态bit，按照以下规则定义
uint16 chargeStartTime = 0;			//记录充电开始的时间 (min)
uint16 sysCount = 0;

//定义电压标准，后期要调整
uint16 adapter_refVoltage = REF_VOLTAGE;
uint16 battery_fullVoltage = REF_BATFULL_VOLTAGE;
uint16 battery_emptyVoltage = REF_BATEMPTY_VOLTAGE;
uint16 battery_needChargeVoltage = REF_BATNEEDCHG_VOLTAGE;
uint16 maxChargeTime = CHARGER_TIME;

/*
Bit		7		6		5		4		3		2		1		Func
									ICH	CHG	PW1	PW0
ICH(is Charge Needed)：	是否正在充电？				0否1是
CHG(Charge)：						电池是否需要充电？		0否1是
PW1&PW0(PowerState):		系统电源？						00市电	01电池	10断电：电池保护

*/

uint8 wakeCount = 0;


RTCTime currentTime = {0,48,20,25,9,18,0};
RTCTime AlarmTime[2] = {{0,55,22,0,0,0,0},{0,0,0,0,0,0,0}};			//初始化值为22：55分的闹钟一个。如果结构体全为0则不设置闹钟。

bool h12,hPM;
DS3231 Clock;				//对象申明

VoltInfo volInfo[3] = {6,"Adapter",0,
											 7,"Charger",0,
											 3,"Battery",0};

uint16 U_Adapter, U_Charger, U_Battery;

void setup(){
	NRF24L01_InitTypeDef NRF24L01_InitStructure;
	
	pinMode(RELAY1_PIN,OUTPUT);
	pinMode(RELAY2_PIN,OUTPUT);
	pinMode(RELAY3_PIN,OUTPUT);
	
	//digitalWrite(RELAY3_PIN,1);				//因为板子原因此功能暂停
	switchToAdapter();
	turnOffCharger();
	
	//NRF模块初始化（包含SPI初始化）
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
	//I2C初始化
  Wire.begin();
	OLED_Config();
	
	OLED_Write_Str16X8(0,0,"SOURCE:");
	OLED_Write_Str16X8(1,0,"Adapter:");
	OLED_Write_Str16X8(2,0,"Charger:");
	OLED_Write_Str16X8(3,0,"Battery:");
	
	//RTC初始化（不包含I2C初始化）
  //RTC一直处于运行当中，无需初始化。所需要的设置仅为时间的设定。
  //此处设置RTC的时钟属性
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
  
  //此处设置RTC的闹钟属性
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
	//此处放采集函数
	getVoltages();
	if(wakeCount >= 8){			//苏醒间隔8秒，八次苏醒读取一次时间，将近一分钟
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
	
	//此处放置所有人机交互界面的相关代码
  /*if(displayFlag){
  	
  	
  	attachInterrupt(digitalPinToInterrupt(2),UPS_Int0_Handler,CHANGE);			//最后将中断使能
  	displayFlag = 0;
  }
  */
  
  //如果有数据传入
  //NRF24L01_PrintState();
	if(NRF24L01_getStatus() & NRF24L01_STATU_RX_DR){
		NRF24L01_recieve(nrfDataBuf);			//接收数据
		
		if(nrfDataBuf[0] == UPS_rxAdd[4] && nrfDataBuf[1] == UPS_rxAdd[3] && nrfDataBuf[2] == UPS_rxAdd[2] && nrfDataBuf[3] == UPS_rxAdd[1] && nrfDataBuf[4] == UPS_rxAdd[0]){
			//赋值
			adapter_refVoltage = ((uint16)nrfDataBuf[5])<<8 | nrfDataBuf[6];
			battery_fullVoltage = ((uint16)nrfDataBuf[7])<<8 | nrfDataBuf[8];
			battery_emptyVoltage = ((uint16)nrfDataBuf[9])<<8 | nrfDataBuf[10];
			maxChargeTime = ((uint16)nrfDataBuf[11])<<8 | nrfDataBuf[12];
			battery_needChargeVoltage = ((uint16)nrfDataBuf[13])<<8 | nrfDataBuf[14];
			
			//存储
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
			
			//以下为测试程序
			OLED_Write_Num16X8(0,10,maxChargeTime);
			OLED_Write_Num16X8(1,10,adapter_refVoltage);
  		OLED_Write_Num16X8(2,10,battery_fullVoltage);
  		OLED_Write_Num16X8(3,10,battery_emptyVoltage);
			OLED_Write_Str16X8(0,10,"OK");
			OLED_Write_Str16X8(1,10,"OK");
			OLED_Write_Str16X8(2,10,"OK");
			OLED_Write_Str16X8(3,10,"OK");
		}
		//清空缓冲区
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


//电源管理逻辑
/*
Bit		7		6		5		4		3		2		1		Func
									ICH	CHG	PW1	PW0
ICH(is Charge Needed)：	是否正在充电？				0否1是
CHG(Charge)：						电池是否需要充电？		0否1是
PW1&PW0(PowerState):		系统电源？						00市电	01电池	10断电：电池保护

*/
void batteryManager(){
	switch(systemState & (UPS_STATE_PW1_MSK | UPS_STATE_PW0_MSK)){
		case 0:				//当前供电为市电
			if(U_Adapter > adapter_refVoltage){		//电源适配器有电
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
			}else if((systemState & UPS_STATE_CHG_MSK) == 0){		//电源适配器没电且电池有电
				switchToBat();
			}else{				//电源适配器没电且电池没电
				
			}
			break;
		case 1:				//当前供电为电池供电
			if(U_Adapter > adapter_refVoltage && ((uint16)currentTime.hour*60 + currentTime.minute) < ((uint16)AlarmTime[0].hour*60 + AlarmTime[0].minute)){
				switchToAdapter();
				//turnOnCharger();			//每次从电池切换过来就自动开启充电器充电（这样充电对电池不好）
				//chargeStartTime = ((uint16)currentTime.hour*60 + currentTime.minute);
			}else if(U_Battery < battery_emptyVoltage){
				switchToAdapter();
				systemState |= 0x06;				//切换至断电模式
				systemState &= ~0x01;
			}else{
				
			}
			break;
		case 2:				//当前为断电情况因电池保护
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
