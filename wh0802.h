#define PORT_WH_EN        GPIOD
#define PIN_WH_EN         5 

#define PORT_WH_RS        GPIOC
#define PIN_WH_RS         3 

#define PORT_WH_DATALINE  GPIOC
#define PIN_WH_d7         7 
#define PIN_WH_d6         6
#define PIN_WH_d5         5
#define PIN_WH_d4         4

#define BIT_en  (1<<PIN_WH_EN)
#define BIT_rs  (1<<PIN_WH_RS)
#define BIT_db7 (1<<PIN_WH_d7)
#define BIT_db6 (1<<PIN_WH_d6)
#define BIT_db5 (1<<PIN_WH_d5)
#define BIT_db4 (1<<PIN_WH_d4)
#define MASK_db (BIT_db7|BIT_db6|BIT_db5|BIT_db4)

#define N_F_code   2 //n:2-line f:5*11font
#define SC_RL_code 2 //sc:setcursor/display moving rl:right/left
#define D_C_B_code 4 //display/cursor/blinking
#define ID_SH_code 2 //cursor mov/display shift

typedef struct {
  unsigned char Be  ; 
  unsigned char Ge  ;
  unsigned char De  ;  
  unsigned char Yo  ;
  unsigned char Zhe ;
  unsigned char Ze  ;
  unsigned char II  ;
  unsigned char ii  ;
  unsigned char Le  ;
  unsigned char Pe  ;
  unsigned char Uu  ;
  unsigned char Fe  ; 
  unsigned char Ce  ;
  unsigned char Che ;
  unsigned char Sha ;
  unsigned char SCHa;
  unsigned char Tb ;
  unsigned char bI  ;
  unsigned char mb ;
  unsigned char EE  ;
  unsigned char yU  ;
  unsigned char yA  ;
  unsigned char deg ;
} Cyr_eeprom_struct;

typedef enum {
  CHRONO_UNIT_NONE, CHRONO_UNIT_HR, CHRONO_UNIT_MIN, CHRONO_UNIT_YEAR, CHRONO_UNIT_MONTH, CHRONO_UNIT_DAY
} ChronoUnit_t;

void del_WH(unsigned int del);
void write_wh(char rs,char db7_db4);
void write_wh_byte(char rs,char db7_db0);
void init1604(void);
void clear(void);
void set_cursor(char x,char y);

void write_wh_digit(char n);
void print_char(char ch, char x, char y);
void print_string(char* pointer, char x, char y); 

void print_xx_number(char number, char x, char y);
void print_xx_number_and_blinker(char number, char x, char y, char blinkerX);
void print_xxx_number(unsigned int number, char x, char y, char blinkerX);
void print_xxxx_number(unsigned int number, char x, char y);
void print_time(char hh, char mm, char ss);
void print_date(char dd, char mm, char yy);
void print_time_and_blinker(char hh, char mm, char ss, char blinker);
void print_date_and_blinker(char dd, char mm, char yy, char blinker);
void print_xx_xx_xx(char lineNumber, char aa, char bb, char cc, char delimiter, char blinker);
void print_xx_xx(char lineNumber, signed char aa, signed char bb, char delimiter, char blinker, char signRequired);
void print_save(void);
void print_greeting(void);
