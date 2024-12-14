
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Stepper.h>

#define RDA 0x80
#define TBE 0x20
#define Type DHT11

int dhtPin = 2, stepsPerRevolution = 2048;
float humidity, temperature;

DHT HT(dhtPin, Type);
Stepper myStepper = Stepper(stepsPerRevolution, 2, 3, 4, 5);

LiquidCrystal lcd(6,7,8,9,10,11);

//ADC Register Initialization 
volatile unsigned char* MY_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* MY_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* MY_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* MY_ADC_DATA = (unsigned int*) 0x78;

//Ports and Pin Initialization
volatile unsigned char* portPinA = (unsigned char*) 0x20;
volatile unsigned char* portA = (unsigned char*) 0x22;
volatile unsigned char* portC = (unsigned char*) 0x28;
volatile unsigned ddrC = (unsigned char*) 0x27;

//UART Register Initialization 
volatile unsigned char* MY_UCSR0A = (unsigned char*)0x00C0;
volatile unsigned char* MY_UCSR0B = (unsigned char*)0x00C1;
volatile unsigned char* MY_UCSR0C = (unsigned char*)0x00C2;
volatile unsigned int* MY_UBRR0 = (unsigned int*)0x00C4;
volatile unsigned char* MY_UDR0 = (unsigned char*)0x00C6;

//Timer Register Initialization 
volatile unsigned char* MY_TCCR1B = (unsigned char*) 0x81;
volatile unsigned int* MY_TCNT1 = (unsigned int*) 0x84;
volatile unsigned char* MY_TIFR1 = (unsigned char*) 0x36;

void setup(){
  U0init(9600);
  adcInit();
  *ddrC &= 0b10101000;
  *portA |= 0b00010101;
}

void loop(){
  humidity = HT.readHumidity();
  temperature = HT.readTemperature();

  int level = adcRead(0);
  if(level >= 300){
    *portC |= 0b01000000;
  }else{
    *portC &= ~0b01000000;
  }

  bool left = (*portC & 0b00010000), right = !(portC & 0b00010000);

  
  if(*portC & 0b00000100){
    *portPinA |= 0b00010000;
  }else{
    *portPinA &= ~0b00010000;
  }

  if(*portC & 0b00000001) {
    *portPinA |= 0b00010000;
  } else {
    *portPinA &= ~0b00010000;
  }

  if (*portC & 0b00000101) {
    *portPinA |= 0b10000000;
  } else {
    *portPinA &= ~0b10000000;
  }

  if(left || right){
    stepps(left,right);
  }

  LCD();
}

void stepps(bool left, bool right){
  myStepper.setSpeed(5);
  if(left){
    myStepper.step(-stepsPerRevolution);
  }

  if(right){
    myStepper.step(stepsPerRevolution);
  }
}

void LCD(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temperature)
  lcd.print("C");
  lcd.print(0,1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
}

void U0init(unsigned long U0baud){
  unsigned long FCPU = 16000000;
  unsigned int tbaud = (FCPU / (16 * U0baud)) - 1;
  *MY_UCSR0A = 0x20;
  *MY_UCSR0B = 0x18;
  *MY_UCSR0C = 0x06;
  *MY_UBRR0  = tbaud;
}

void adcInit(){
  //setting up A register
  *my_ADCSRA |= 0b10000000; 
  *my_ADCSRA &= 0b10111111; 
  *my_ADCSRA &= 0b11011111; 
  *my_ADCSRA &= 0b11011111; 

  //setting up B register
  *MY_ADCSRB &= 0b11110111;
  *MY_ADCSRB &= 0b11111000; 

  //setting up mux register
  *MY_ADMUX  &= 0b01111111; 
  *MY_ADMUX  |= 0b01000000; 
  *MY_ADMUX  &= 0b11011111;
  *MY_ADMUX  &= 0b11011111; 
  *MY_ADMUX  &= 0b11100000; 
}

unsigned int adcRead(unsigned char adcChannelNum){
  *_ADMUX = (0x40 | adcChannelNum); 
  *my_ADCSRA = 0x87;
  while ((*MY_ADCSRA) & (1 << ADSC)) {}
  int adcValue = *MY_ADC_DATA;
  *my_ADCSRA = 0x00; 
  return adcValue;
}

void myDelay(unsigned int freq) {
  double period = 1.0/double(freq);
  double half_period = period/ 2.0f;
  double clk_period = 0.0000000625;
  unsigned int ticks = half_period / clk_period;
  *MY_TCCR1B &= 0xF8;
  *MY_TCNT1 = (unsigned int) (65536 - ticks);
  *MY_TCCR1B |= 0b00000001;
  while((*MY_TIFR1 & 0x01)==0); 
  *MY_TCCR1B &= 0xF8;   
  *MY_TIFR1 |= 0x01;
}

unsigned char U0kbhit(){
  return(*my_UCSR0A & RDA);
}

unsigned char U0getChar(){
  return *MY_UDR0;
}

void U0putChar(unsigned char U0pData){
  while((*MY_UCSR0A &= TBE) == 0){};
  *MY_UDR0 = U0pData;
}



