#include "stm8s.h"

#define PORT_BUTTON_OK      GPIOD

#define PORT_BUTTON_UP_DOWN GPIOD


#define pin_btn_down      1
#define bit_btn_down                       (1<<pin_btn_down)
#define pin_btn_up        3
#define bit_btn_up                         (1<<pin_btn_up)
#define pin_btn_ok        2
#define bit_btn_ok                         (1<<pin_btn_ok)
 

typedef struct {
	int cnt_up_btn;
	int cnt_down_btn;
	int cnt_ok_btn;
	char f_push :1;
	char f_long_push :1;
        
	int up_down_btn_data;
	int up_down_btn_data_previous;
	int up_down_btn_data_lim_h;
	int up_down_btn_data_lim_l;
	char ok_btn_data;
	char ok_btn_data_previous;
	char ok_btn_data_lim_h;
	
	unsigned int transient_counter;
	char tick;
	char tick_previous;
	char count_down_timer;
} Buttons_t;

@far @interrupt void TIM2Interrupt (void);
void initTim2 (void);
void initButtons(Buttons_t* btns);
void buttonHandler(Buttons_t* btns);
void setupUpDownButtonLimits(int lim_l, int lim_h, int first_value);
void setupOkButtonLimits(char lim_h, char first_value);
