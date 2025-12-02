#ifndef RTC_H
#define RTC_H

#define SECOND_PER_DAY  86400
#define SECOND_MOSCOW_ALIGNMENT  (3*3600)
#define FIRST_YEAR      1970
#define LEAP_DAYS       366
#define NON_LEAP_DAYS   365

#define PORT_RTC_RST GPIOA
#define PORT_RTC_DAT GPIOA
#define PORT_RTC_CLK GPIOA

#define PIN_dat_rtc     2   
#define PIN_clk_rtc     3   
#define PIN_rst_rtc     1   


#define BIT_dat_rtc    (1<<PIN_dat_rtc)
#define BIT_clk_rtc    (1<<PIN_clk_rtc)
#define BIT_rst_rtc    (1<<PIN_rst_rtc)


#define SEC   0
#define MIN   1
#define HR    2
#define DATE  3
#define MONTH 4
#define DAY   5
#define YEAR  6
#define CONTR 7
#define TR_CH 8
#define WHOLE 3



typedef struct {
  int year;
  char month;
  char dayOfMonth;
} LocalDate; 

typedef struct {
  char hr;
  char min;
  char sec;
} LocalTime; 

typedef struct {
  unsigned long epochSec;
  char sec;
  
  unsigned long epochSecToMimute;
  unsigned int minFromMidnight;
  
  unsigned long epochSecToDay;
  char day;
  
  char cacheEneble;
} SecondsRtcUtcCache;

typedef struct {
  unsigned int rawDays;
  LocalDate date;
} DateFromEpochDaysCache;

typedef struct {
  unsigned long hoursFromFirst;
  unsigned long shiftSeconds;
	char cacheEneble;
} AlignmentTimeCache;

typedef struct {
  unsigned long epochRawSeconds;
  unsigned long epochSecToDay;
  char cacheEneble;
} TimeTransferBodyCache;

typedef struct {
  unsigned long epochSecFirstPoint;
  unsigned long shiftSeconds;
  char timeCorrSec;
  char positiveCorr;
  char timeCorrDecaMs;
  char cachedSeconds;
} TimeAlignment; 

typedef struct {
  int year;
  char month;
  char dayOfMonth;
  char hr;
  char min;
  char sec;
} TransferBody; 




void delay_rtc(char del );
void rtc_set_time_date(LocalTime time, LocalDate date);
void init_rtc(void);
void send_byte_rtc(char data);
char receive_byte_rtc(void);
void send_rtc(char reg_adr, char data);
char receive_rtc(char reg_adr);
char receive_plain_val_rtc(char reg_adr);

char getMonthLength(char month, char leap);
char isLeapYear(int year);
LocalDate getMonth(int days, int year);
LocalDate getDateFromEpochDays(unsigned int days);
unsigned long getEpochDaysOfDate(char year, char month, char day);
unsigned long getDays(unsigned long seconds);
unsigned long receiveEpochSecondsRtcMoscow(void);
void refreshTimeTransferBody(void);
unsigned long getActualSeconds(unsigned long epochRawSec);

#endif