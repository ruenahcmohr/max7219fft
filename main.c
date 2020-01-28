/*

avr @ 16Mhz
9600 baud



on pro mini:

0   rxd    rxd
1   txd    txd

2   pd2    
3   pd3    


4   pd4    keyb. R1  row inputs
5   pd5    keyb. R2
6   pd6    keyb. R3
7   pd7    keyb. R4

8   pb0    keyb. C1   column outputs
9   pb1    keyb. C2
10  pb2    keyb. C3


11  pb4    LCD RS
12  pb3
13  pb5    LCD /E


A0  pc0    LCD D0
A1  pc1    LCD D1
A2  pc2    LCD D2
A3  pc3    LCD D3

A4  pc4    wheel +
A5  pc5    wheel -


A6  adc6  
A7  adc7  



*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "MAX7219.h"
#include "avrcommon.h"

#define OUTPUT  1
#define INPUT   0

uint8_t RBI_FSM32[] = { 16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,8,9,10,11,12,13,14,15,4,5,6,7,2,3,1,0 };

int8_t sinTable32[] = { -0.000000,-6.242890,-12.245871,-17.778248,-22.627417,-26.607027,-29.564144,-31.385128,-32.000000,-31.385128,-29.564146,-26.607029,-22.627417,-17.778246,-12.245872,-6.242890};
int8_t cosTable32[] = { 32.000000,31.385128,29.564144,26.607027,22.627417,17.778248,12.245870,6.242891,-0.000001,-6.242890,-12.245869,-17.778246,-22.627417,-26.607029,-29.564144,-31.385130};

volatile int8_t real[32];
volatile int8_t imag[32];

volatile char ADCFlag;

volatile uint8_t  IRQIDX ;


static void AnalogInit (void);
static void timerInit(void) ;

#define BITSIZE 5
#define nn (1<<BITSIZE)

int main( void ) {
 
 int a; //, d; //, i;
 
    int8_t     d, i, j, b, n;
    
    int8_t        wr, wi;
    int       tempr, tempi;

    // set up directions 
  DDRB = (OUTPUT << PB0 | OUTPUT << PB1 | INPUT << PB2 | INPUT << PB3 | INPUT << PB4 |OUTPUT << PB5 | INPUT << PB6 | INPUT << PB7);
  DDRD = (INPUT << PD0 | INPUT << PD1 | OUTPUT << PD2 |OUTPUT << PD3 |OUTPUT << PD4 |OUTPUT << PD5 |OUTPUT << PD6 |OUTPUT << PD7);        
  DDRC = (INPUT << PC0 | INPUT << PC1 | INPUT << PC2 |INPUT << PC3 |INPUT << PC4 |INPUT << PC5 |INPUT << PC6 ); 
  
  IRQIDX = 0;
  
  AnalogInit();
  timerInit();
  max7219Init( );
  
  sei();
 
   ADCFlag = 0;
   while(ADCFlag == 0);
   ADCFlag = 0;
 
  while(1) {
   // wait for buffer
   
   while(ADCFlag == 0);    
   
   for ( i = 0; i < 32; i++) imag[i] = 0;
      
   for(d = 1, b = BITSIZE-1; d < nn; d <<= 1, b-- ) {         
    n = -1;                                           // fraction initialization, this will immediatly have 1 added to it by the if         
    for(i = (nn-1), j = 0; j != (nn-1); i += (d << 1) ) {            // this will always itterate nn/2 times
      if ((i >= nn) || (j == 0)) {                             // do calcs and wrap the i index back into the array with an offset
        i -= (nn-1);   n++;                         // dont use i %= (nn-1);  its MUCH slower, otherwise, loop stuff               
        wr = cosTable32[n<<b];
        wi = sinTable32[n<<b];
      }          
      j = i + d;   // j is just an offset of i     
  
      tempr     =  (wr * real[j])/32 - (wi * imag[j])/32;    // defacto standard complex multiply
      tempi     =  (wr * imag[j])/32 + (wi * real[j])/32;   
      real[j]   =  real[i] - tempr; // update real, crossed path calculations
      imag[j]   =  imag[i] - tempi; // update imaginary 
      real[i]   += tempr;            // update real, forward path calculations
      imag[i]   += tempi;            // update imaginary      
      
    }           
  }
   
   // process data
   for ( i = 0; i < 8; i++) {    
     a = ((int)ABS(real[i+3])+(int)ABS(imag[i+3]))>>5;
     
     d = (1<<a)-1;
     send16(max7219MakePacket(cmdDIG0+(7-i), d));    
   }
   
   // start new buffer
   ADCFlag = 0;
   TIFR1 |= (1<<TOV1); // restart adc
    
  }
}

// 7112Hz
void timerInit(void) {

// Fast pwm mode, overflows @ OCR1A 
  TCCR1A |= (1<<WGM10)|(1<<WGM11);
  // set prescaler to /1 and FAST PWM mode
  TCCR1B |= (1<<CS10)|(1<<WGM12)|(1<<WGM13);
  
  // about 7111.11 hz
  OCR1A = 2250;

}


void AnalogInit (void) {  

  // auto trigger on timer 1 overflow, how cool is that!
  ADCSRB = 1 << ADTS2 |
           1 << ADTS1 ;

  // Activate ADC with Prescaler of 128, yielding max of 9.6ksps
  ADCSRA =  1 << ADEN  |
            0 << ADSC  | 
            1 << ADATE | /* auto start when timer1 overflows */
            0 << ADIF  |
            1 << ADIE  | /* enable interrupt */
            1 << ADPS2 |
            1 << ADPS1 |
            1 << ADPS0 ;
                        
  ADMUX = (1<<REFS0)|(1<<ADLAR);     // channel 0  , shift right 2 places for us   
  
}


ISR(ADC_vect) { 
  
  static uint8_t i;
  
  SetBit(5, PINB);
  
  if (!ADCFlag) { 
    real[IRQIDX] = ADCH;  // save value
  
    IRQIDX = RBI_FSM32[IRQIDX];

    if (IRQIDX == 0) { 
    ADCFlag = 1;
   }
  
    TIFR1 |= (1<<TOV1);
  } 
  return;
}

















