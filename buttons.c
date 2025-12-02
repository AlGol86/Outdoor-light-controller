#include "stm8s.h"
#include "buttons.h"


void initTim2 (void) {//encoder_poller
  TIM2->IER|=TIM2_IER_UIE;
	//TIM2->IER|=TIM1_IER_CC1IE;
  TIM2->PSCR=0;
	TIM2->ARRH=10; //2
  TIM2->ARRL=20;//20
	TIM2->CNTRH=0;
	TIM2->CNTRL=0;
	rim();
	TIM2->CR1|=TIM2_CR1_CEN;
  }


void initButtons(Buttons_t* btns){
	
	initTim2();
	
	PORT_BUTTON_UP_DOWN->CR2 &=  ~(bit_btn_down|bit_btn_up);
	PORT_BUTTON_UP_DOWN->DDR &=  ~(bit_btn_down|bit_btn_up);
	PORT_BUTTON_UP_DOWN->CR1 |=  (bit_btn_down|bit_btn_up);
	PORT_BUTTON_OK->CR2   &=  ~bit_btn_ok;
	PORT_BUTTON_OK->DDR   &=  ~bit_btn_ok;
	PORT_BUTTON_OK->CR1   |=  bit_btn_ok;
	
  btns->ok_btn_data_lim_h=1;        
}


void buttonHandler(Buttons_t* btns){ 
  char cnt_lim       = 20;
	int cnt_long_lim = 5000;

//down button
	if ((PORT_BUTTON_UP_DOWN->IDR & bit_btn_down)==0) {
		if (btns->cnt_down_btn < cnt_long_lim) {
			btns->cnt_down_btn++;
	  } 
  } else	if (btns->cnt_down_btn > 0) {
		if (btns->cnt_down_btn > cnt_lim) {
			btns->cnt_down_btn = cnt_lim;
	  } 
	  btns->cnt_down_btn--;
	}
	
//up button
	if ((PORT_BUTTON_UP_DOWN->IDR & bit_btn_up)==0) {
		if (btns->cnt_up_btn < cnt_long_lim) {
			btns->cnt_up_btn++;
	  } 
  } else	if (btns->cnt_up_btn > 0) {
		if (btns->cnt_up_btn > cnt_lim) {
			btns->cnt_up_btn = cnt_lim;
	  } 
	  btns->cnt_up_btn--;
	}
	
//ok button
	if ((PORT_BUTTON_OK->IDR & bit_btn_ok)==0) {
		if (btns->cnt_ok_btn < cnt_long_lim) {
			btns->cnt_ok_btn++;
	  } 
  } else	if (btns->cnt_ok_btn > 0) {
		if (btns->cnt_ok_btn > cnt_lim) {
			btns->cnt_ok_btn = cnt_lim;
	  } 
	  btns->cnt_ok_btn--;
	}

//ok button short	
	if ((btns->cnt_ok_btn > (cnt_lim - 10)) && (btns->f_push == 0)) {
		btns->f_push = 1;
		if (btns->ok_btn_data < btns->ok_btn_data_lim_h) btns->ok_btn_data++;
		else btns->ok_btn_data = 0;
	}		

//up button short	
  if ((btns->cnt_up_btn > (cnt_lim - 10)) && (btns->f_push == 0)) {
		btns->f_push = 1;
		if (btns->up_down_btn_data < btns->up_down_btn_data_lim_h) btns->up_down_btn_data++;
		else btns->up_down_btn_data = btns->up_down_btn_data_lim_l;
	}
//down button short
	if ((btns->cnt_down_btn > (cnt_lim - 10)) && (btns->f_push == 0)) {
		btns->f_push = 1;
		if (btns->up_down_btn_data > btns->up_down_btn_data_lim_l) btns->up_down_btn_data--;
		else btns->up_down_btn_data = btns->up_down_btn_data_lim_h;
	}
	
//up button long	
  if ((btns->cnt_up_btn == cnt_long_lim) && (btns->up_down_btn_data < btns->up_down_btn_data_lim_h)) {
		btns->up_down_btn_data++;
		btns->cnt_up_btn -= 800;
	}
//down button long	
	if ((btns->cnt_down_btn == cnt_long_lim) && (btns->up_down_btn_data > btns->up_down_btn_data_lim_l)) {
		btns->up_down_btn_data--;
		btns->cnt_down_btn -= 800;
	}
	
//released buttons
	if ((btns->cnt_up_btn == 0) && (btns->cnt_down_btn == 0) &&  (btns->cnt_ok_btn == 0))	{
	  btns->f_push = 0;
		btns->f_long_push = 0;
	}
	
}
	
void setupUpDownButtonLimits(int lim_l, int lim_h, int first_value){
		
		extern Buttons_t buttons;	
		
		buttons.up_down_btn_data_lim_l = lim_l;
		buttons.up_down_btn_data_lim_h = lim_h;
		buttons.up_down_btn_data       = first_value;
		buttons.up_down_btn_data_previous = first_value;
}

void setupOkButtonLimits(char lim_h, char first_value) {
	
		extern Buttons_t buttons;	
		
		buttons.ok_btn_data_lim_h = lim_h;
		buttons.ok_btn_data       = first_value;
		buttons.ok_btn_data_previous = first_value;
}