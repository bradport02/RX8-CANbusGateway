/*
 * Brad Port
 * EE3000
 * 03/04/26
 *
 */

#include "main.h"
#include "time.h"
#include "communication/protocol.h"

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

uint8_t format = 0; //24 hour
time_t timeBuf = {0};

void Time_SendTime(UART_HandleTypeDef *huart){
	Time_GetTime();
    timeBuf.hours = sTime.Hours;
    timeBuf.minutes = sTime.Minutes;
    timeBuf.seconds = sTime.Seconds;
    timeBuf.day = sDate.Date;
    timeBuf.month = sDate.Month;
    timeBuf.year = sDate.Year;
    timeBuf.format = 0;

    uint8_t data[7];
    data[0] = timeBuf.hours;
    data[1] = timeBuf.minutes;
	data[2] = timeBuf.seconds;
	data[3] = timeBuf.day;
	data[4] = timeBuf.month;
	data[5] = timeBuf.year;
	data[6] = timeBuf.format;


    Protocol_Send(huart, STM, CMD_SENDTIME, data, sizeof(data));

}

void Time_GetTime(){
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);  // must read date too or time locks
}
