#include "WDTwake.h"

void WDTwake_init(){
	/*** Setup the WDT ***/
	MCUSR &= ~(1<<WDRF); /* 清除复位标志. */
	/* In order to change WDE or the prescaler, we need to
	 * set WDCE (This will allow updates for 4 clock cycles).
	 */
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	/* 设置新的看门狗超时时间 */
	//WDTCSR = 1<<WDP1 | 1<<WDP2; /* 1.0 seconds */
	WDTCSR = 1<<WDP3 | 1<<WDP0; /* 8.0 seconds */
	/* 设置为定时中断而不是复位 */
	WDTCSR |= _BV(WDIE);
}

void WDTwake_enterSleep(){
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
	sleep_enable();
	sleep_mode();/* Now enter sleep mode. */
	/* The program will continue from here after the WDT timeout*/
	sleep_disable(); /* First thing to do is disable sleep. */
	power_all_enable();/* Re-enable the peripherals. */
}

ISR(WDT_vect){//看门狗唤醒执行函数
	
}

