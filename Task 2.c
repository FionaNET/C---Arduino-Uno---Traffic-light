/*
Task 2:
Develop your basic traffic-light system (Task 1) so that it is now configurable. The
configurable system can be set to operate in one of four states.
*/

//variable setup:
//variable to keep track of the colour 1 = red, 2 = green, 3 = orange
volatile uint8_t colour = 1;
//global variable to control the lights
volatile uint8_t flag = 1;
//variable to store 10bit digital word from the potentiometer
volatile uint16_t resultADC; 
//varible to toggle: 1 is config mode, 0 is off config mode
volatile uint8_t button = 0; 
//variable to store the state of the lights; default = 1
volatile uint8_t state = 1; 
//stores the number of times the blue LED should blink depending on state
volatile uint8_t blink = 0; 

void setup()
{
  //disable global interrupts
  cli();
  
  DDRB |= (1 << DDB2); //red
  DDRB |= (1 << DDB1); //yellow
  DDRB |= (1 << DDB0); //green
  DDRC |= (1 << DDC1); //blue
  DDRD &= ~(1 << DDD2); //set Pin 2 as input for the switch
  
  //enable external interrupt on PORTD2
  EIMSK |= (1<<INT0); //external interrupt mask register
  //sense a rising edge (when both ISC01 and ISC00 is set to 1
  EICRA |= (1<< ISC01) | (1<<ISC00); //external interrupt control register A 
  
  //initialize timer/counter 1 and control registers on normal mode  
  TCCR1A = 0;  
  TCCR1B = 0;
  TCNT1 = 0; 
  
  //prescaler of 1024
  TCCR1B |= (1<<CS10) | (0 <<CS11) | (1<<CS12); 
  //low count to immediately go into ISR COMPA
  OCR1A = 1; 
  //enable Timer1 compare interrupt A
  TIMSK1 |= (1<<OCIE1A);
  
  //Blue on until configuration mode
  PORTC |= (1<<PC1); 

  //setting up ADC
  ADMUX = 0x0;
  ADMUX |= (1<<REFS0) | (0<<REFS1);
  ADCSRA = 0x0;
  //enable the ADC and the conversion complete interrupt
  ADCSRA |= (1<<ADEN)|(1<<ADIE);
  
  //enable global interrupts
  sei();
  
  //program works when print is enabled
  Serial.begin(9600);
}

void loop()
{ 
  //calculate the state when config mode is entered
  if (button == 1) {
    //start ADC conversion.
 	ADCSRA |= (1<<ADSC); 
    
    //0-255 = state 1
    if (resultADC < 256) {
      state = 1;
    //256 - 511 = state 2
    } else if ((resultADC > 255) && (resultADC < 512)) {
      state = 2;
    //512 - 767 = state 3
    } else if ((resultADC > 511) && (resultADC < 768)) {
      state = 3;
    //768 - 1023 = state 4
    } else {
      state = 4;
    }
    
  }
  
  delay(10);
}


ISR(TIMER1_COMPA_vect) {  
  
  //when the button is pressed and the red LED is lit we can enter config mode
  if ((button == 1) && (colour == 1)) {
    
    //printing makes progam work properly
    Serial.println(state);
    
    //keep red LED on
    PORTB |= (1<<PB2);
	
    //blue LED pulse has a period of 1/state with a 50% duty cycle
    if (blink < (state*2)) {
      OCR1A = 15624/(state*2);
      
      //when flag = 1, it turns on LED
      if (flag == 1) {
        PORTC |= (1<<PC1);
        flag = 0;
        
      //when flag = 0, it turns off LED
      } else {
        PORTC &= ~(1<<PC1);
        flag = 1;
      }
      //increment blink
      blink += 1;
    
    //when blue LED has blinked the number of state, it is turned off for 2s
    } else {
      PORTC &= ~(1<<PC1);
      OCR1A = 31248;
      
      //reset for the next round of pulses
      blink = 0;
    }
    
  //normal mode
  } else {
  
    //when flag = 1, turn on a LED light
    if (flag == 1) {
      
      //how long the LED light is on depends on the state
      OCR1A = (15624*state);
      
      //when red should light up
      if (colour == 1) {
          PORTB |= (1<<PB2);
      //when green should light up
      } else if (colour == 2) {
          PORTB |= (1<<PB0);
      //when yellow should light up
      } else {
          PORTB |= (1<<PB1);
      }

      flag = 0;
      
    //when flag = 0, turn off a LED light
    } else {
      
      //should immediately turn on the next light
      OCR1A = 1;
      
      //when red should turn off
      if (colour == 1) {
          PORTB &= ~(1<<PB2);
          //next colour is green
          colour = 2;
      //when green should turn off
      } else if (colour == 2) {
          PORTB &= ~(1<<PB0);
          //next colour is yellow
          colour = 3;
      //when yellow should turn off
      } else {
          PORTB &= ~(1<<PB1);
          //next colour is red
          colour = 1;
      }

      flag = 1;
    }
  }
  //reset Timer1
  TCNT1 = 0;
}


//ISR conversion complete interrupt
ISR(ADC_vect) { 
  //obtain the 10bit word
  resultADC = ADC;    
}


//interrupt for the button
ISR(INT0_vect) {
  //toggle the button
  button ^= 1;
  
  //immediately go into config mode once red light is reached
  if (colour == 1) {
    OCR1A = 1;
    TCNT1 = 0;
  }
  
}


