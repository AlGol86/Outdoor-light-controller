#include "rtc.h"

#define PORT_LED                GPIOB
#define PIN_TIMER_LED           4 
#define BIT_TIMER_LED          (1 << PIN_TIMER_LED)
#define PIN_LIGHT_SENSOR_LED    5 
#define BIT_LIGHT_SENSOR_LED   (1 << PIN_LIGHT_SENSOR_LED)
#define BITMASK_LED   (BIT_TIMER_LED | BIT_LIGHT_SENSOR_LED)

#define PORT_RELAY              GPIOD
#define PIN_RELAY               4 
#define BIT_RELAY              (1 << PIN_RELAY )

#define LIGHT_SENSOR_CH         6



typedef struct {
	      char epochSecFirstPoint[4];
        char timeCorrSec;
        char timeCorrDecaMs;
        char positiveCorr;
				unsigned int turnOnMinutesFromMidnight;
				unsigned int turnOffMinutesFromMidnight;
				unsigned int turnOnLightLevel;
				unsigned int turnOffLightLevel;
} Eeprom_data_t;

typedef enum {
  STATE_OFF = 0, 
	STATE_ON = 1
} LedState_t;

typedef enum {
  TIMER_LED        = BIT_TIMER_LED,
  LIGHT_SENSOR_LED = BIT_LIGHT_SENSOR_LED
} LedSelector_t;

typedef enum {time} menu_t;

typedef enum {
  set_hr, set_min, set_year, set_month, set_day, set_int_corr, set_fract_corr, set_light_level_on, set_light_level_off, set_timer_on, set_timer_off
} Set_selector_t;

void initLed(void);
void controlLed(LedSelector_t which, LedState_t state);
unsigned int get_ADC(char ch);


char scanDateTimeValues(Set_selector_t set_selector, LocalTime presetTime, LocalDate presetDate);
signed char scanCorrection(Set_selector_t set_selector, int correction);
char scan_xx_xx_values(Set_selector_t set_selector);
char scanTimer(Set_selector_t set_selector, int timerInMinutesFromMidnight);
unsigned int scanLightLevel(void);
void setupDateTime(void);
void setupDateTimeCorrection(void);
void setupRelayControlParameters(Set_selector_t set_selector);

void saveFirstPointTimeToEeprom(void);
void saveTimeCorrectionToEeprom(void);
void saveRelayControlParameters(unsigned int turnOnMinutesFromMidnight, unsigned int turnOffMinutesFromMidnight, unsigned int turnOnLightLevel, unsigned int turnOffLightLevel);
void populate_timeAlignment_from_eeprom(void);
void initRelay(void);
void enableRelay(void);
void disableRelay(void);
int clearScreenIfMenuChanged(void);
