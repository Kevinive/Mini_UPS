#ifndef __UPS_CONFIG_H
#define __UPS_CONFIG_H
#include "Include.h"

#define uint8			unsigned char
#define uint16		unsigned int
#define uint32		unsigned long
#define int8			char
#define int16			int
#define int32			long

#define K1_PIN					2
#define K2_PIN					4
#define RELAY1_PIN			16
#define RELAY2_PIN			15
#define RELAY3_PIN			14
#define SAMPLE1_PIN			6			//12.5=850
#define SAMPLE2_PIN			7			//12.5=850
#define SAMPLE3_PIN			3			//12.5=850
#define OLED_SCL				8
#define OLED_SDA				7

#define OLED_ADDR				0X78
#define EEPROM_ADDR			0X57

#define ALM1BIT_HMS			0X08
#define ALM2BIT_HMS			0X40

#define RXBUFSIZE 			32

#define REF_VOLTAGE							800					//阈值电压AD值，低于此则认为电压为0 否则有电（仅用于Charger&Adapter）
#define REF_BATEMPTY_VOLTAGE		720					//阈值电压AD值，低于此认为电池没电，关断输出，并将需要充电标志置位。
#define REF_BATFULL_VOLTAGE			1000					//阈值电压AD值，充电过程中高于此认为电池充满，结束充电过程。
#define CHARGER_TIME						360					//充电时间（经验值）(min)
#define REF_BATNEEDCHG_VOLTAGE	811

#define UPS_STATE_ICH_MSK				0X08
#define UPS_STATE_CHG_MSK				0X04
#define UPS_STATE_PW1_MSK				0X02
#define UPS_STATE_PW0_MSK				0X01

typedef struct DS3231time{
	uint8 second;
	uint8 minute;
	uint8 hour;
	uint8 date;
	uint8 month;
	uint8 year;
	uint8 DoW;
} RTCTime;

typedef struct VoltageInfo{
	uint8 pin;
	const uint8 *displayName;
	uint16 value;
} VoltInfo;

extern RTCTime currentTime;
extern RTCTime AlarmTime[2];

extern VoltInfo volInfo[3];
extern uint16 U_Adapter, U_Charger, U_Battery;

extern void getVoltages();

#endif