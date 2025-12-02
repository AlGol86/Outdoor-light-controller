//this example is intended for 8 MHz(?) clock frequency
#include "stm8s.h"
#include "rtc.h"

void delay_rtc(char del) {
  char i;
  for(i=0;i<(del);i++) {nop();}
}

void init_rtc(void) {
  PORT_RTC_CLK->CR2&=~BIT_clk_rtc ; // low speed (interrupt disabled)
  PORT_RTC_RST->CR2&=~BIT_rst_rtc ; // low speed (interrupt disabled)
  PORT_RTC_CLK->CR1|=BIT_clk_rtc ; // p-p 
  PORT_RTC_RST->CR1|=BIT_rst_rtc ; // p-p 
  PORT_RTC_CLK->ODR&=~BIT_clk_rtc ;  //clk_pin, rst_pin  - 0 
  PORT_RTC_RST->ODR&=~BIT_rst_rtc ;  //clk_pin, rst_pin  - 0 
  PORT_RTC_CLK->DDR|=BIT_clk_rtc ; 
  PORT_RTC_RST->DDR|=BIT_rst_rtc ; 
   
  PORT_RTC_DAT->CR2&=~BIT_dat_rtc; //dat_pin  - low speed (interrupt disabled)
  PORT_RTC_DAT->CR1|=BIT_dat_rtc; //dat_pin - p-p /float in inp. direct
  PORT_RTC_DAT->ODR&=~BIT_dat_rtc;  //dat_pin  - 1 (SDA)
  PORT_RTC_DAT->DDR|=BIT_dat_rtc; 
 }

void rtc_set_time_date(LocalTime time, LocalDate date) {
  send_rtc(CONTR,0);  //WRITE-PROTECT: OFF
  send_rtc(TR_CH,0);    //TRICKLE CHARGER: OFF
  send_rtc(YEAR, date.year % 100); 
  //send_rtc(DAY,  WEEK_INIT);    //DAY OF WEEK
  send_rtc(MONTH, date.month); 
  send_rtc(DATE, date.dayOfMonth); 
  send_rtc(HR, time.hr); 
  send_rtc(MIN, time.min); 
  send_rtc(SEC, 0);
}

void send_byte_rtc(char data) {
	char i;
  for(i=1;i<=8;i++) {
    PORT_RTC_DAT->ODR|=(data&1)<<PIN_dat_rtc;
    data>>=1;
    PORT_RTC_CLK->ODR|=BIT_clk_rtc; // clk=1
    nop();
    PORT_RTC_DAT->ODR&=~BIT_dat_rtc; // sda=1
    PORT_RTC_CLK->ODR&=~BIT_clk_rtc;// clk=0
  }
}

char  receive_byte_rtc(void) {
  char data=0;
	char i;
  PORT_RTC_DAT->CR1|=BIT_dat_rtc;
  nop();
  PORT_RTC_DAT->DDR&=~BIT_dat_rtc;
  for(i=0;i<=7;i++) {
    nop();
    PORT_RTC_CLK->ODR|=BIT_clk_rtc; // clk=1
    nop();
    data|=((PORT_RTC_DAT->IDR&BIT_dat_rtc)>>PIN_dat_rtc)<<i;
    nop();
    PORT_RTC_CLK->ODR&=~BIT_clk_rtc;// clk=0
    nop();
    }
    PORT_RTC_DAT->CR1|=BIT_dat_rtc;
    PORT_RTC_DAT->DDR|=BIT_dat_rtc;
    return (data);
}

void send_rtc(char reg_adr, char data) { 
  data = ((data/10)<<4) | data%10;
  delay_rtc(3); //9*0.5us //3
  PORT_RTC_RST->ODR|=BIT_rst_rtc; //rst=1(>4us)
  delay_rtc(3); //9*0.5us  //3
  reg_adr<<=1;
  reg_adr+=0x80;
  send_byte_rtc(reg_adr);
  nop();
  send_byte_rtc(data);
  PORT_RTC_RST->ODR&=~BIT_rst_rtc; //rst=0
}

char receive_rtc(char reg_adr) {
  char receive_data;
  delay_rtc(3); //9*0.5us //3
  PORT_RTC_RST->ODR|=BIT_rst_rtc; //rst=1(>4us)
  delay_rtc(3); //9*0.5us  //3
  reg_adr<<=1;
  reg_adr+=0x81;
  send_byte_rtc(reg_adr);
  receive_data=receive_byte_rtc();
  PORT_RTC_RST->ODR&=~BIT_rst_rtc; //rst=0
  return receive_data;
}

char receive_plain_val_rtc(char reg_adr) {
  char val;
  val = receive_rtc(reg_adr);
  return ((val & 0xf0)>>4) * 10 + (val & 0x0f);
}


/*************************************************
*                 TIME-ENHANCEMENT               *
*************************************************/
 
unsigned long receiveEpochSecondsRtcMoscow() {
  
  extern SecondsRtcUtcCache secondsRtcUtcCache;
  unsigned int currentMinFromMidnight;
	char dayOfMonth;
  char currentSec = receive_plain_val_rtc(SEC);
  if(currentSec == secondsRtcUtcCache.sec && secondsRtcUtcCache.cacheEneble) {
    return secondsRtcUtcCache.epochSec;
  }
  secondsRtcUtcCache.sec = currentSec;
  currentMinFromMidnight = ((unsigned int)receive_plain_val_rtc(HR) * 60) + receive_plain_val_rtc(MIN); 
   if(currentMinFromMidnight == secondsRtcUtcCache.minFromMidnight && secondsRtcUtcCache.cacheEneble) {
    secondsRtcUtcCache.epochSec =  secondsRtcUtcCache.epochSecToMimute + currentSec;
    return secondsRtcUtcCache.epochSec;
  }
  secondsRtcUtcCache.minFromMidnight = currentMinFromMidnight;
  
  dayOfMonth = receive_plain_val_rtc(DATE);
  if(dayOfMonth == secondsRtcUtcCache.day && secondsRtcUtcCache.cacheEneble) {
     secondsRtcUtcCache.epochSecToMimute = secondsRtcUtcCache.epochSecToDay + ((unsigned long)currentMinFromMidnight) * 60;
     secondsRtcUtcCache.epochSec = secondsRtcUtcCache.epochSecToMimute + currentSec;
     return secondsRtcUtcCache.epochSec;
  }
  secondsRtcUtcCache.day = dayOfMonth;
  
  secondsRtcUtcCache.epochSecToDay = getEpochDaysOfDate(receive_plain_val_rtc(YEAR), receive_plain_val_rtc(MONTH), dayOfMonth) * SECOND_PER_DAY;
  secondsRtcUtcCache.epochSecToMimute = secondsRtcUtcCache.epochSecToDay + ((unsigned long)currentMinFromMidnight) * 60;
  secondsRtcUtcCache.epochSec = secondsRtcUtcCache.epochSecToMimute + currentSec;
  secondsRtcUtcCache.cacheEneble = 1;
  return secondsRtcUtcCache.epochSec;
}

unsigned long getEpochDaysOfDate(char year, char month, char day) {
  unsigned long result = 0;
	int i;
	
  for (i = FIRST_YEAR; i < (year + 2000); i++) {
    result += isLeapYear(i) ? LEAP_DAYS : NON_LEAP_DAYS;
  }
  for (i = 1; i < month; i++) {
    result += getMonthLength(i, isLeapYear(year + 2000));
  }
  return result + day - 1;;
}



void refreshTimeTransferBody(void) {  
  
  extern TransferBody transferBody; 
  extern TimeTransferBodyCache timeTransferBodyCache;
	
	unsigned long actual_now;
  LocalDate date;
  unsigned long epochRawSec = receiveEpochSecondsRtcMoscow();
	
  if (timeTransferBodyCache.epochRawSeconds == epochRawSec && timeTransferBodyCache.cacheEneble) {
    return;
  }
  timeTransferBodyCache.epochRawSeconds = epochRawSec;
  
  actual_now =  getActualSeconds(epochRawSec);    
  
  date = getDateFromEpochDays(actual_now / SECOND_PER_DAY);
  transferBody.dayOfMonth = date.dayOfMonth;
  transferBody.month      = date.month;
  transferBody.year       = date.year;  
  
  transferBody.sec = actual_now % 60;
  actual_now /= 60; 
   
  transferBody.min = actual_now % 60;
  actual_now /= 60;  
   
  transferBody.hr  = actual_now % 24;  
  
  timeTransferBodyCache.cacheEneble = 1;
 }

unsigned long getActualSeconds(unsigned long epochRawSec) {

  extern TimeAlignment timeAlignment;
  extern AlignmentTimeCache alignmentTimeCache;

  unsigned long secFromFirst = epochRawSec - timeAlignment.epochSecFirstPoint;
  
  unsigned long hoursFromFirst = secFromFirst / 3600;
  unsigned long daysFromFirst;
	unsigned int correctionDecaMsPerDay;
	
  if (alignmentTimeCache.hoursFromFirst != hoursFromFirst || !alignmentTimeCache.cacheEneble) {
    alignmentTimeCache.hoursFromFirst = hoursFromFirst;
    daysFromFirst = hoursFromFirst / 24;
    correctionDecaMsPerDay = (unsigned int) timeAlignment.timeCorrSec * 100 + timeAlignment.timeCorrDecaMs; 
    timeAlignment.shiftSeconds = daysFromFirst * (correctionDecaMsPerDay) / 100;
    timeAlignment.shiftSeconds += ((secFromFirst % SECOND_PER_DAY) * (correctionDecaMsPerDay)) / 3600 / 2400;
    alignmentTimeCache.shiftSeconds = timeAlignment.shiftSeconds;
  }
  alignmentTimeCache.cacheEneble = 1;
  if (timeAlignment.positiveCorr) return epochRawSec + timeAlignment.shiftSeconds;
  return epochRawSec - timeAlignment.shiftSeconds;
}

LocalDate getDateFromEpochDays(unsigned int days) {
  
   extern DateFromEpochDaysCache dateFromEpochDaysCache;
   int year = FIRST_YEAR;
   unsigned int dayCntr = 0;
   unsigned int dayInYear = 0; 
   int currentDays;	 
	 LocalDate date;
	 
   if (days == dateFromEpochDaysCache.rawDays) {
     return dateFromEpochDaysCache.date;
   }
   dateFromEpochDaysCache.rawDays = days;
   

   while (dayCntr + dayInYear <= days) {
     dayCntr += dayInYear;
     dayInYear = isLeapYear(year) ? LEAP_DAYS : NON_LEAP_DAYS;
     year++;
   }
   year--;
   currentDays = days - dayCntr + 1;
   
   date.year = year;
   date.month = getMonth(currentDays, year).month;
   date.dayOfMonth = getMonth(currentDays, year).dayOfMonth;
   dateFromEpochDaysCache.date = date;
   return date;
}  

LocalDate getMonth(int days, int year) {
   char month = 2;
   int dayCntr = 0;
   int dayInMonth = 31;
	 int currentDays;
	 LocalDate date;
	 
   while (dayCntr + dayInMonth < days) {
     dayCntr += dayInMonth;
     dayInMonth = getMonthLength(month, isLeapYear(year));
     month++;
   }
   month--;
   currentDays = days - dayCntr;
   
   date.month = month;
   date.dayOfMonth = currentDays;
   return date;
}

char isLeapYear(int year) {
   return (year % 400) == 0 || (year % 100) != 0 && (year % 4) == 0;
}

char getMonthLength(char month, char leap) {

        switch (month) {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12: return 31;
            case 4:
            case 6:
            case 9:
            case 11: return 30;
            case 2: return leap ? 29 : 28;
            default: return -1;
        }
}
