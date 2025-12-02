#include "stm8s.h"
#include "main.h"
#include "wh0802.h"
#include "rtc.h"
#include "buttons.h"


AlignmentTimeCache     alignmentTimeCache;
TransferBody           transferBody; 
TimeTransferBodyCache  timeTransferBodyCache;
SecondsRtcUtcCache     secondsRtcUtcCache;
DateFromEpochDaysCache dateFromEpochDaysCache;

TimeAlignment          timeAlignment;

Buttons_t              buttons;

int                    currentLightLevel;

Eeprom_data_t          eeprom_data     @0x4100;         //store setting variables (in EEPROM)()
@far Cyr_eeprom_struct s_cyr_eeprom[8] @0x4010;   //cyr_alphabet

char timerConditionDetected;
char lightLevelConditionDetected;


main() {
	int currentTimeMinutesFromMidnight;
	
	CLK->CKDIVR = 0x00; //~ 16 MHz   << 0x11 - 2MHz  0x10 - 4MHz  0x01 - 8MHz  0x00 - 16MHz >>
	
	del_WH(20000);
	init1604();
	initLed();
	init_rtc();
	initRelay();
  populate_timeAlignment_from_eeprom();
	initButtons(&buttons);
	setupOkButtonLimits(1, 0);
	setupUpDownButtonLimits(0, 6, 0);
	
	print_greeting();
	
	while (1) {
		
		refreshTimeTransferBody();
		
		
		if (buttons.tick_previous ^ buttons.tick) {
			//measure light and trigger light level condition
			buttons.tick_previous = buttons.tick;
			currentLightLevel = get_ADC(LIGHT_SENSOR_CH);
			if (currentLightLevel < eeprom_data.turnOnLightLevel) {
			  lightLevelConditionDetected = 1;
				controlLed(LIGHT_SENSOR_LED, STATE_ON);
		  } else if (currentLightLevel > eeprom_data.turnOffLightLevel) {
			  lightLevelConditionDetected = 0;
				controlLed(LIGHT_SENSOR_LED, STATE_OFF);
			}
			
			//check timer condition
			currentTimeMinutesFromMidnight = transferBody.hr * 60 + transferBody.min;
			if (eeprom_data.turnOffMinutesFromMidnight < eeprom_data.turnOnMinutesFromMidnight) {
				if (currentTimeMinutesFromMidnight >= eeprom_data.turnOnMinutesFromMidnight || currentTimeMinutesFromMidnight <  eeprom_data.turnOffMinutesFromMidnight) {
					timerConditionDetected = 1;
					controlLed(TIMER_LED, STATE_ON);
			  } else  {
				  timerConditionDetected = 0;
					controlLed(TIMER_LED, STATE_OFF);
			  }
		  } else if (eeprom_data.turnOnMinutesFromMidnight < eeprom_data.turnOffMinutesFromMidnight) {
			  if (currentTimeMinutesFromMidnight >= eeprom_data.turnOnMinutesFromMidnight && currentTimeMinutesFromMidnight <  eeprom_data.turnOffMinutesFromMidnight) {
					timerConditionDetected = 1;
					controlLed(TIMER_LED, STATE_ON);
			  } else  {
				  timerConditionDetected = 0;
					controlLed(TIMER_LED, STATE_OFF);
			  }
			} else {
				timerConditionDetected = 0;
				controlLed(TIMER_LED, STATE_OFF);
			}
			
			// colttol relay
			if (timerConditionDetected && lightLevelConditionDetected) {
				enableRelay();
			} else {
				disableRelay();
			}
			
		}
		
	  clearScreenIfMenuChanged();
		
		switch(buttons.up_down_btn_data) {
		  case 1:
			case 2:
			print_char('(', 4, 2);
			print_xxx_number(currentLightLevel, 5, 2, 0);
			print_char(')', 8, 2);
			break;
		}
		
	  switch(buttons.up_down_btn_data) {
		  case 0: //time and date
			print_time(transferBody.hr, transferBody.min, transferBody.sec);
			print_date(transferBody.year % 100, transferBody.month, transferBody.dayOfMonth);	
      if (buttons.ok_btn_data) {
				setupOkButtonLimits(6, 0);
				setupDateTime();
				setupUpDownButtonLimits(0, 6, 0);
      }			
			break;
		  
			case 1: //light level - on
			print_string("ON", 1, 1);
      print_xxx_number(eeprom_data.turnOnLightLevel, 1, 2, 0);
			if (buttons.ok_btn_data) {
				setupOkButtonLimits(1, 0);
				setupRelayControlParameters(set_light_level_on);
				setupOkButtonLimits(1, 0);
				setupUpDownButtonLimits(0, 6, 1);
      }		
			break;
			case 2: //light level - off
			print_string("OFF", 1, 1);
      print_xxx_number(eeprom_data.turnOffLightLevel, 1, 2, 0);
			if (buttons.ok_btn_data) {
				setupOkButtonLimits(1, 0);
				setupRelayControlParameters(set_light_level_off);
				setupOkButtonLimits(1, 0);
				setupUpDownButtonLimits(0, 6, 2);
      }		
			break;
			case 3: //timer - on
			print_string("ON", 1, 1);
      print_xx_xx(2, eeprom_data.turnOnMinutesFromMidnight / 60, eeprom_data.turnOnMinutesFromMidnight % 60, ':', 0, 0);
			if (buttons.ok_btn_data) {
				setupOkButtonLimits(1, 0);
				setupRelayControlParameters(set_timer_on);
				setupOkButtonLimits(1, 0);
				setupUpDownButtonLimits(0, 6, 3);
      }		
			break;
			case 4: //timer - off
			print_string("OFF", 1, 1);
      print_xx_xx(2, eeprom_data.turnOffMinutesFromMidnight / 60, eeprom_data.turnOffMinutesFromMidnight % 60, ':', 0, 0);
			if (buttons.ok_btn_data) {
				setupOkButtonLimits(1, 0);
				setupRelayControlParameters(set_timer_off);
				setupOkButtonLimits(1, 0);
				setupUpDownButtonLimits(0, 6, 4);
      }		
			break;
		  
			case 5:		
			print_string("CORR:", 1, 1);
      print_xx_xx(2, timeAlignment.positiveCorr ? timeAlignment.timeCorrSec : -timeAlignment.timeCorrSec, timeAlignment.positiveCorr ? timeAlignment.timeCorrDecaMs : -timeAlignment.timeCorrDecaMs , '.', 0, 1);
			if (buttons.ok_btn_data) {
				setupOkButtonLimits(1, 0);
				setupDateTimeCorrection();
				setupOkButtonLimits(1, 0);
				setupUpDownButtonLimits(0, 6, 5);
      }		
			break;
			
			case 6:		
			print_string("STAT:", 1, 1);
			print_xxxx_number(((receiveEpochSecondsRtcMoscow() - timeAlignment.epochSecFirstPoint) / 24 / 3600) % 10000, 1, 2);
			break;
	  }
	}
}

int clearScreenIfMenuChanged(){
	if (buttons.up_down_btn_data != buttons.up_down_btn_data_previous | buttons.ok_btn_data != buttons.ok_btn_data_previous) {
		buttons.up_down_btn_data_previous = buttons.up_down_btn_data;
		buttons.ok_btn_data_previous = buttons.ok_btn_data;
	  clear();
	} 
}

void setupRelayControlParameters(Set_selector_t set_selector) {
	unsigned int turnOnLightLevel;
	unsigned int turnOffLightLevel;
	unsigned int turnOnMinutesFromMidnight;
	unsigned int turnOffMinutesFromMidnight;

	turnOnLightLevel = eeprom_data.turnOnLightLevel > 994 ? 994 : eeprom_data.turnOnLightLevel;
	turnOffLightLevel = eeprom_data.turnOffLightLevel > 999 ? 999 : eeprom_data.turnOffLightLevel;
	turnOnMinutesFromMidnight = eeprom_data.turnOnMinutesFromMidnight > 1439 ? 1439 : eeprom_data.turnOnMinutesFromMidnight;
	turnOffMinutesFromMidnight = eeprom_data.turnOffMinutesFromMidnight > 1439 ? 1439 : eeprom_data.turnOffMinutesFromMidnight;

	switch(set_selector) {
			
			case set_light_level_on:
        setupUpDownButtonLimits(0, turnOffLightLevel - 5, turnOnLightLevel);			
        turnOnLightLevel = scanLightLevel();			
				break;
				
			case set_light_level_off:	
        setupUpDownButtonLimits(turnOnLightLevel + 5, 999, turnOffLightLevel);			
			  turnOffLightLevel = scanLightLevel();
				break;
				
			case set_timer_on:	
        setupUpDownButtonLimits(0, 23, turnOnMinutesFromMidnight / 60);			
			  turnOnMinutesFromMidnight = scanTimer(set_hr, turnOnMinutesFromMidnight) * 60 + (turnOnMinutesFromMidnight % 60);
				setupUpDownButtonLimits(0, 59, turnOnMinutesFromMidnight % 60);			
			  turnOnMinutesFromMidnight = (turnOnMinutesFromMidnight / 60) * 60 + scanTimer(set_min, turnOnMinutesFromMidnight) ;
				break;
				
			case set_timer_off:	
        setupUpDownButtonLimits(0, 23, turnOffMinutesFromMidnight / 60);			
			  turnOffMinutesFromMidnight = scanTimer(set_hr, turnOffMinutesFromMidnight) * 60 + (turnOffMinutesFromMidnight % 60);
				setupUpDownButtonLimits(0, 59, turnOffMinutesFromMidnight % 60);			
			  turnOffMinutesFromMidnight = (turnOffMinutesFromMidnight / 60) * 60 + scanTimer(set_min, turnOffMinutesFromMidnight) ;
				break;
			
	}
	
	if(turnOnLightLevel != eeprom_data.turnOnLightLevel || turnOffLightLevel != eeprom_data.turnOffLightLevel || turnOnMinutesFromMidnight != eeprom_data.turnOnMinutesFromMidnight || turnOffMinutesFromMidnight != eeprom_data.turnOffMinutesFromMidnight) {
		print_save(); 
		saveRelayControlParameters(turnOnMinutesFromMidnight, turnOffMinutesFromMidnight, turnOnLightLevel, turnOffLightLevel);
	}
	
}

void setupDateTime() {
	LocalTime presetTime;
	LocalDate presetDate;
	
	//set time	
	presetTime.hr = transferBody.hr;
	presetTime.min = transferBody.min;
	
	setupUpDownButtonLimits(0, 23, presetTime.hr);
	
	presetTime.hr = scanDateTimeValues(set_hr, presetTime, presetDate);
	
	setupUpDownButtonLimits(0, 59, presetTime.min);
	presetTime.min = scanDateTimeValues(set_min, presetTime, presetDate);
	
	
	//set date
	presetDate.dayOfMonth = transferBody.dayOfMonth;
	presetDate.month = transferBody.month;
  presetDate.year = transferBody.year;	

	setupUpDownButtonLimits(00, 99, presetDate.year % 100);
  presetDate.year = scanDateTimeValues(set_year, presetTime, presetDate) + 2000;
	
	setupUpDownButtonLimits(1, 12, presetDate.month);
	presetDate.month = scanDateTimeValues(set_month, presetTime, presetDate);

	setupUpDownButtonLimits(1, 31, presetDate.dayOfMonth);
	presetDate.dayOfMonth = scanDateTimeValues(set_day, presetTime, presetDate);
  
	//save date and time if changed
	if(presetDate.dayOfMonth != transferBody.dayOfMonth || presetDate.month != transferBody.month || presetDate.year != transferBody.year || presetTime.hr != transferBody.hr || presetTime.min != transferBody.min) {
		rtc_set_time_date(presetTime, presetDate);
    secondsRtcUtcCache.cacheEneble = 0;
		timeAlignment.epochSecFirstPoint = receiveEpochSecondsRtcMoscow();
		saveFirstPointTimeToEeprom();
		print_save(); 
	}
}

void setupDateTimeCorrection() {
	int correction;
	signed char intPart;
	signed char fraction;
	TimeAlignment presetCorrection;

	//set correction
	presetCorrection.timeCorrSec = timeAlignment.timeCorrSec;
	presetCorrection.timeCorrDecaMs = timeAlignment.timeCorrDecaMs;
	presetCorrection.positiveCorr = timeAlignment.positiveCorr;
	
  correction = presetCorrection.timeCorrSec * 100 + presetCorrection.timeCorrDecaMs;
	correction = presetCorrection.positiveCorr ? correction : -correction;
	setupUpDownButtonLimits(-99, 99, correction/100);
	intPart = scanCorrection(set_int_corr, correction);
	presetCorrection.timeCorrSec = (intPart < 0) ? -intPart : intPart;
	setupUpDownButtonLimits((intPart == 0) ? -99 : 0, 99, (intPart != 0 & correction < 0) ? -correction % 100 : correction % 100);
	fraction = (intPart < 0) ? -scanCorrection(set_fract_corr, intPart*100) : scanCorrection(set_fract_corr, intPart*100);
	presetCorrection.timeCorrDecaMs = (fraction < 0) ? -fraction : fraction;
	presetCorrection.positiveCorr = (fraction < 0 || intPart < 0) ? 0 : 1;
	
  //save correction if changed
	if(presetCorrection.timeCorrSec != timeAlignment.timeCorrSec || presetCorrection.timeCorrDecaMs != timeAlignment.timeCorrDecaMs || presetCorrection.positiveCorr != timeAlignment.positiveCorr) {
		print_save(); 
		timeAlignment.timeCorrSec = presetCorrection.timeCorrSec;
		timeAlignment.timeCorrDecaMs = presetCorrection.timeCorrDecaMs;
		timeAlignment.positiveCorr = presetCorrection.positiveCorr;
		saveTimeCorrectionToEeprom();		
		alignmentTimeCache.cacheEneble = 0;
	}
}

char scanDateTimeValues(Set_selector_t set_selector, LocalTime presetTime, LocalDate presetDate) {
	while(buttons.ok_btn_data == 0) {
		switch(set_selector) {
			
			case set_hr:	
        print_time_and_blinker(buttons.up_down_btn_data, presetTime.min, 0, buttons.tick ? 1 : 0);			
				break;
			case set_min:		
			  print_time_and_blinker(presetTime.hr, buttons.up_down_btn_data, 0, buttons.tick ? 4 : 0);	
				break;
				
			case set_year:
			  print_date_and_blinker(buttons.up_down_btn_data, presetDate.month, presetDate.dayOfMonth, buttons.tick ? 1 : 0);				
				break;
			case set_month:			
				print_date_and_blinker(presetDate.year % 100, buttons.up_down_btn_data, presetDate.dayOfMonth, buttons.tick ? 4 : 0);		
				break;
			case set_day:		
				print_date_and_blinker(presetDate.year % 100, presetDate.month, buttons.up_down_btn_data, buttons.tick ? 7 : 0);		
				break;
		}
	}
	buttons.ok_btn_data = 0;
	return buttons.up_down_btn_data;
}

signed char scanCorrection(Set_selector_t set_selector, int correction) {
	while(buttons.ok_btn_data == 0) {
	  switch(set_selector) {
			case set_int_corr:	
				print_xx_xx(2, buttons.up_down_btn_data, (correction % 100), '.', buttons.tick ? 2 : 0, 1);
				break;
			case set_fract_corr:		
			  print_xx_xx(2, (correction / 100), buttons.up_down_btn_data, '.', buttons.tick ? 5 : 0, 1); 	
				break;
		}
	}
	
	buttons.ok_btn_data = 0;
	return buttons.up_down_btn_data;
}

char scanTimer(Set_selector_t set_selector, int timerInMinutesFromMidnight) {
	while(buttons.ok_btn_data == 0) {
	  switch(set_selector) {
			case set_hr:	
				print_xx_xx(2, buttons.up_down_btn_data, (timerInMinutesFromMidnight % 60), ':', buttons.tick ? 1 : 0, 0);
				break;
			case set_min:		
			  print_xx_xx(2, (timerInMinutesFromMidnight / 60), buttons.up_down_btn_data, ':', buttons.tick ? 4 : 0, 0); 	
				break;
		}
	}
	
	buttons.ok_btn_data = 0;
	return buttons.up_down_btn_data;
}

unsigned  scanLightLevel() {
	while(buttons.ok_btn_data == 0) {
	  print_xxx_number(buttons.up_down_btn_data, 1, 2, buttons.tick ? 1 : 0);
	}
	buttons.ok_btn_data = 0;
	return buttons.up_down_btn_data;
}

void initLed() {
  PORT_LED -> CR2 &= ~BITMASK_LED;
	PORT_LED -> CR1 &= ~BITMASK_LED;
	PORT_LED -> DDR |=  BITMASK_LED;
	PORT_LED -> ODR |=  BITMASK_LED;
}

void initRelay() {
  PORT_RELAY -> CR2 &= ~BIT_RELAY;
	PORT_RELAY -> CR1 |=  BIT_RELAY;
	PORT_RELAY -> DDR |=  BIT_RELAY;
	PORT_RELAY -> ODR &= ~BIT_RELAY;
}

void enableRelay() {
  PORT_RELAY -> ODR |= BIT_RELAY;
}

void disableRelay() {
 PORT_RELAY -> ODR &= ~BIT_RELAY;
}
	
void controlLed(LedSelector_t which, LedState_t state) {
  if (state) PORT_LED -> ODR &= ~which;
	else       PORT_LED -> ODR |=  which;
}

unsigned int get_ADC(char ch) {
  unsigned int h=0;
  char l=0;
  ADC1->CSR=ADC1_CSR_CH&ch;
  ADC1->CR1|=ADC1_CR1_ADON; 
  ADC1->CR1|=ADC1_CR1_ADON;  //dubble 'ADON" for switch on ADC
  while((ADC1->CSR&ADC1_CSR_EOC)==0) nop();

  ADC1->CSR&=~ADC1_CSR_EOC;
  h=ADC1->DRH;
  l=ADC1->DRL;
	h = ((h<<2)|l);
  return h > 999 ? 999 : h;
}



void saveTimeCorrectionToEeprom() { 
  char i;
	sim();
  if (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) {  // unlock EEPROM
       FLASH->DUKR = 0xAE;
       FLASH->DUKR = 0x56;
  }
  rim();
  while (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) ;
 

  eeprom_data.timeCorrSec = timeAlignment.timeCorrSec;
  eeprom_data.timeCorrDecaMs = timeAlignment.timeCorrDecaMs;
  eeprom_data.positiveCorr = timeAlignment.positiveCorr;
  
  FLASH->IAPSR &= ~(FLASH_IAPSR_DUL);  // lock EEPROM
}  

void saveFirstPointTimeToEeprom() { 
  char i;
	sim();
  if (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) {  // unlock EEPROM
       FLASH->DUKR = 0xAE;
       FLASH->DUKR = 0x56;
  }
  rim();
  while (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) ;
  
  for(i =0; i<4; i++) {
    eeprom_data.epochSecFirstPoint[i] =  (timeAlignment.epochSecFirstPoint >> (i * 8)) & 0xFF;
  }
    
  FLASH->IAPSR &= ~(FLASH_IAPSR_DUL);  // lock EEPROM
} 

void saveRelayControlParameters(unsigned int turnOnMinutesFromMidnight, unsigned int turnOffMinutesFromMidnight, unsigned int turnOnLightLevel, unsigned int turnOffLightLevel) { 
	sim();
  if (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) {  // unlock EEPROM
       FLASH->DUKR = 0xAE;
       FLASH->DUKR = 0x56;
  }
  rim();
  while (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) ;
	
	eeprom_data.turnOnMinutesFromMidnight =  turnOnMinutesFromMidnight;
	eeprom_data.turnOffMinutesFromMidnight = turnOffMinutesFromMidnight;
	eeprom_data.turnOnLightLevel =           turnOnLightLevel;
	eeprom_data.turnOffLightLevel =          turnOffLightLevel;
    
  FLASH->IAPSR &= ~(FLASH_IAPSR_DUL);  // lock EEPROM
} 


void populate_timeAlignment_from_eeprom() { 
  char i;
  for(i = 0; i<4; i++) {
     timeAlignment.epochSecFirstPoint |=  ((unsigned long) eeprom_data.epochSecFirstPoint[i]) << (i * 8);
  }
  timeAlignment.timeCorrSec = eeprom_data.timeCorrSec;
  timeAlignment.timeCorrDecaMs = eeprom_data.timeCorrDecaMs;
  timeAlignment.positiveCorr = eeprom_data.positiveCorr;
} 

