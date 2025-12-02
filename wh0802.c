#include "stm8s.h"
#include "wh0802.h"

//+..8.75k...|..1.23k..- contrast balance or +..10k..|..2*680R..-

void del_WH(unsigned int del) {
	while(del--);
}

void init1604() {
  PORT_WH_DATALINE->ODR&=~(MASK_db);//b00001111
  PORT_WH_DATALINE->DDR|=(MASK_db); //b11110000
  PORT_WH_DATALINE->CR1|=(MASK_db); //b11110000
  PORT_WH_DATALINE->CR2|=(MASK_db); //b11110000
  PORT_WH_RS->ODR&=~BIT_rs;
  PORT_WH_EN->ODR&=~BIT_en;
  PORT_WH_RS->DDR|=BIT_rs; 
  PORT_WH_EN->DDR|=BIT_en;
  PORT_WH_RS->CR1|=BIT_rs; 
  PORT_WH_EN->CR1|=BIT_en;   
  PORT_WH_RS->CR2&=~BIT_rs; 
  PORT_WH_EN->CR2&=~BIT_en;
  del_WH(60000);
  write_wh_byte(0,8);
  write_wh(0,0x30); //arg: char rs, char db7_db4
  del_WH(100);
  write_wh_byte(0,32|N_F_code<<2);
  write_wh_byte(0,32|N_F_code<<2);
  write_wh_byte(0,8|(D_C_B_code));
  clear();
  write_wh_byte(0,4|ID_SH_code);
  clear();
  set_cursor(1,1);
}

void clear(void) {
  write_wh_byte(0,1);
  del_WH(4000);
}

void set_cursor(char x,char y) {
  x--;
  switch(y){
    case 1: {write_wh_byte(0,0x80|0x0|x);break;} 
    case 2: {write_wh_byte(0,0x80|0x40|x);break;}
  }
}

void write_wh(char rs,char db7_db4) { // rs=0-instruction; rs=1-data (write half of byte)
  PORT_WH_DATALINE->ODR&=~MASK_db;
  PORT_WH_RS->ODR&=~BIT_rs;
  PORT_WH_RS->ODR|=rs<<PIN_WH_RS;
  PORT_WH_EN->ODR|=BIT_en;
  PORT_WH_DATALINE->ODR|=(db7_db4&0xF0);
  nop();
  PORT_WH_EN->ODR&=~BIT_en;
}

void write_wh_byte(char rs,char db7_db0) {
  write_wh(rs,db7_db0);
  write_wh(rs,db7_db0<<4);
  PORT_WH_DATALINE->ODR&=~(MASK_db);
  del_WH(40);
}

void write_wh_digit(char n) {
  write_wh_byte(1,0x30 + n);
}

void print_char(char ch, char x, char y) {
  set_cursor(x,y);
  write_wh_byte(1,ch);
}

void print_string(char* pointer, char x, char y) { //print eng_litters string
  set_cursor(x,y);
  while (*pointer!=0){
    write_wh_byte(1,*pointer);
    pointer++;
  }
}

void print_xx_number(char number, char x, char y) {
  set_cursor(x,y);
  write_wh_digit(number / 10);
	write_wh_digit(number % 10);
}

void print_xx_number_and_blinker(char number, char x, char y, char blinkerX) {
  set_cursor(x,y);
  if (blinkerX == x) {
		print_char('_', x, y);
	} else {
	  write_wh_digit(number / 10);
  }
	write_wh_digit(number % 10);
}

void print_xxx_number(unsigned int number, char x, char y, char blinkerX) {
  set_cursor(x,y);
	if (blinkerX == x) {
    print_char('_', x, y);
	} else {
		write_wh_digit(number / 100);
	}
	write_wh_digit((number % 100) /10);
	write_wh_digit(number % 10);
}


void print_xxxx_number(unsigned int number, char x, char y) {
  set_cursor(x,y);
	write_wh_digit(number / 1000);
	write_wh_digit((number % 1000) / 100);
	write_wh_digit((number % 100) /10);
	write_wh_digit(number % 10);
}

void print_time(char hh, char mm, char ss) {
	print_xx_xx_xx(1, hh, mm, ss, ':', 0);
}

void print_date(char yy, char mm, char dd) {
	print_xx_xx_xx(2, yy, mm, dd, '-', 0);
}

void print_time_and_blinker(char hh, char mm, char ss, char blinker) {
	print_xx_xx_xx(1, hh, mm, ss, ':', blinker);
}

void print_date_and_blinker(char yy, char mm, char dd, char blinker) {
	print_xx_xx_xx(2, yy, mm, dd, '-', blinker);
}

void print_xx_xx_xx(char lineNumber, char aa, char bb, char cc, char delimiter, char blinker) {
	print_xx_number_and_blinker(aa, 1, lineNumber, blinker);
	print_char(delimiter, 3, lineNumber);
	print_xx_number_and_blinker(bb, 4, lineNumber, blinker);
	print_char(delimiter, 6, lineNumber);
	print_xx_number_and_blinker(cc, 7, lineNumber, blinker);
}

void print_xx_xx(char lineNumber, signed char aa, signed char bb, char delimiter, char blinker, char signRequired) {
	char x = 1;
	if (signRequired & ((aa == 0 & bb >= 0) |  aa > 0)) print_char('+', x++, lineNumber);
	if (aa < 0 || (aa == 0 & bb < 0)) print_char('-', x++, lineNumber);
	print_xx_number_and_blinker((aa < 0) ? -aa : aa, x, lineNumber, blinker);
	print_char(delimiter, (x + 2), lineNumber);
	print_xx_number_and_blinker((bb < 0) ? -bb : bb, (x + 3), lineNumber, blinker);
}

void print_save() {
	char i=12;
	clear();
	print_string("Saving..", 1, 1);
	while (i--) del_WH(65000);
	clear();
}

void print_greeting() {
	char i=15;
	clear();
	print_string("Labas!", 1, 1);
	while (i--) del_WH(65000);
	clear();
	i=25;
	print_string("Laikas", 1, 1);
	print_string("veikti!", 1, 2);
	while (i--) del_WH(65000);
	clear();
}


/*
char print_n_01(unsigned long int n, char x, char y ) { //n=0..65535 right align x-right position
  do {
    print_char((0x30+(n%10)),x,y); 
    n/=10;
    x--;
  } while(n > 0); 
  return (x);
}
*/

/*      
char print_n_10(unsigned int n, char x, char y ) { //n=0..65535 left align x-left position
  unsigned int tn=0;
  char fz=0;
  if ((n%10)==0)  {n++;fz=1;}
  if(n==1) print_char(0x30,x,y); 
  else {
    while(n>0) {tn=(tn*10)+(n%10); n/=10;}
    while(tn>9) {print_char(0x30+(tn%10),x,y); tn/=10;x++;}
    if(fz) print_char(0x30,x,y); else print_char(0x30+tn,x,y);
  }
  x++;
  return (x);
}
*/

/*
void print_n_00(unsigned long int n, char nn, char x, char y ) { //n=0..65535 fixed format - nn diges x-left pos
  char pos;  
  pos = print_n_01(n,x+nn-1,y);
  while(pos >= x) { print_char(0x30,pos,y); pos--;}
}
*/

// arg n_CGRAM = 1..8 (it is a number in LCD memory to set character) so - <=8 cyr chars could be on screen at the same time  
//use if LCD does not provide CYR letters
/*
char print_symb(@far unsigned char* ch_cyr_pointer,char x,char y,char n_CGRAM) {             
  char j=0;
	
  write_wh_byte(0,64|(n_CGRAM<<3)); //set address CGRAM_segment
  for (j=0;j<8;j++) { //no.string pix in character
        write_wh_byte(1,*ch_cyr_pointer);  //send data[0..7] to CGRAM_segment
        ch_cyr_pointer += sizeof(Cyr_eeprom_struct);
	}
  set_cursor(x,y);
  write_wh_byte(1,n_CGRAM);
  n_CGRAM++; 
  return n_CGRAM;
}
*/

/*
void print_n_10_format(unsigned int n, char x, char y, char format ) //n=0..65535 left align x-left position, format-number of patterns
{
  char pos;
  pos = print_n_10(n,x,y);
  while(pos < (x+format)) {
    write_wh_byte(1,' ');
    pos++;
	}
}
*/

/*
void send_cyr_to_eeprom() {
	
 @far extern Cyr_eeprom_struct s_cyr_eeprom[8];

  Cyr_eeprom_struct data_input[8]={
   {0x1f,0x1f,0x0e,0x12,0x15,0x1e,0x11,0x0a,0x1c,0x1f,0x11,0x04,0x12,0x11,0x15,0x15,0x18,0x11,0x10,0x0e,0x12,0x0f,0x18},
   {0x11,0x11,0x0a,0x00,0x15,0x01,0x11,0x04,0x14,0x11,0x11,0x0e,0x12,0x11,0x15,0x15,0x08,0x11,0x10,0x11,0x15,0x11,0x18},
   {0x10,0x10,0x0a,0x1e,0x15,0x01,0x13,0x11,0x12,0x11,0x11,0x15,0x12,0x11,0x15,0x15,0x08,0x11,0x10,0x01,0x15,0x11,0x00},
   {0x1e,0x10,0x0a,0x10,0x0e,0x0e,0x15,0x13,0x11,0x11,0x0a,0x15,0x12,0x0f,0x15,0x15,0x0e,0x19,0x1e,0x07,0x1d,0x0f,0x07},
   {0x11,0x10,0x0a,0x1c,0x15,0x01,0x19,0x15,0x11,0x11,0x04,0x15,0x12,0x01,0x15,0x15,0x09,0x15,0x11,0x01,0x15,0x05,0x04},
   {0x11,0x10,0x1f,0x10,0x15,0x01,0x11,0x19,0x11,0x11,0x08,0x0e,0x12,0x01,0x15,0x15,0x09,0x15,0x11,0x11,0x15,0x09,0x04},
   {0x1e,0x10,0x11,0x1e,0x15,0x1e,0x11,0x11,0x11,0x11,0x10,0x04,0x1f,0x01,0x1f,0x1f,0x0e,0x19,0x1e,0x0e,0x12,0x11,0x07},
   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
  };
	//be   ge   de   yo  zhe   ze   II   ii   le   pe   uu   fe   ce   che  sha scha  tb   Ib   mb   ee   yu   ya   deg

  char i,j=0;

	
  sim();
	if (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))){  // unlock EEPROM
    FLASH->DUKR = 0xAE;
    FLASH->DUKR = 0x56;
	}
  rim();
  
  while (!((FLASH->IAPSR) & (FLASH_IAPSR_DUL))) {;}
	
	for (i = 0; i < 8; i++) {
	  unsigned char *src = (unsigned char *)&data_input[i];
    @far unsigned char *dst = (@far unsigned char *)&s_cyr_eeprom[i];
    for (j = 0; j < sizeof(Cyr_eeprom_struct); j++) {
      dst[j] = src[j];
    }
  }
 
  FLASH->IAPSR &= ~(FLASH_IAPSR_DUL); // lock EEPROM
}
*/

/*
example() {
	  del_WH(50000);
	  init1604();
	  //send_cyr_to_eeprom();
		//print_char(0x54,1,1);//"T="
		//print_char(0x3d,2,1);

		print_symb(&s_cyr_eeprom[0].Be,1,1,1);
		print_symb(&s_cyr_eeprom[0].Ge,2,1,2);
		print_symb(&s_cyr_eeprom[0].De,3,1,3);
		print_symb(&s_cyr_eeprom[0].Yo,4,1,4);
		print_symb(&s_cyr_eeprom[0].Zhe,5,1,5);
		print_symb(&s_cyr_eeprom[0].Ze,6,1,6);
		print_symb(&s_cyr_eeprom[0].II,7,1,7);
		print_symb(&s_cyr_eeprom[0].ii,8,1,8);
		//print_string("CTPOKA.:",1,2);
		print_string("+- ^<$:+",1,2);
	  //print_symb(&s_cyr_eeprom[0].Le,1,2,1);
	  //print_symb(&s_cyr_eeprom[0].Pe,2,2,2);
		//print_symb(&s_cyr_eeprom[0].Uu,3,2,3);
		//print_symb(&s_cyr_eeprom[0].Fe,4,2,4);
		//print_symb(&s_cyr_eeprom[0].Ce,5,2,5);
		//print_symb(&s_cyr_eeprom[0].Che,6,2,6);
		//print_symb(&s_cyr_eeprom[0].Sha,7,2,7);
		//print_symb(&s_cyr_eeprom[0].SCHa,8,2,8);
}
*/