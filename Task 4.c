/*
Task 4:
Implement a red-light camera. The traffic light cycles as normal (i.e., State 1). The camera
is triggered if the traffic light is red, and a vehicle breaches light barrier LB3.
*/

//variable setup:
//set to 1 when LB3 has been pressed and reset to 0 once blinking is done
volatile uint8_t LB3 = 0;
//counts how many times the camera has been triggered
volatile float breachCount = 0; 
//variable to keep track of the colour 1 = red, 2 = green, 3 = orange
volatile uint8_t colour = 1; 
//global variable to control the lights (1 = turn on; 0 = turn off)
volatile uint8_t flag = 1; 
//global variable to control the blue LED (1 = turn on; 0 = turn off)
volatile uint8_t flagBlue = 1;
//count how many times the blue light has blinked
volatile uint8_t blink = 0; 
//counts how many times the timer goes into overflow for the traffic lights
volatile uint8_t overflow1 = 0;
//counts how many times the timer goes into overflow for the blue LED
volatile uint8_t overflow2 = 0;

void setup() 
{
  //disable interrupts
  cli();
  
  //setting up input/outputs
  DDRB |= (1 << DDB2); //set Pin 10 as output (red)
  DDRB |= (1 << DDB1); //set Pin 9 as output (yellow)
  DDRB |= (1 << DDB0); //set Pin 8 as output (green)
  DDRC |= (1 << DDC4); //set Pin A4 as output (blue)
  DDRB |= (1 << DDB5); //set Pin 13 as output (oscilloscope)
  DDRD &= ~(1 << DDD2); //set Pin 2 as input (button)
  
  //set Timer1 to normal mode
  TCCR1A = 0;  
  TCCR1B = 0;
  TCNT1 = 0; 
  //set Timer2 to normal mode
  TCCR2A = 0;  
  TCCR2B = 0;
  TCNT2 = 0; 
  
  //setting prescaler to 1024 for Timer1
  TCCR1B |= (1<<CS10) | (0 <<CS11) | (1<<CS12); 
  //setting prescaler to 1024 for Timer2
  TCCR2B |= (1<<CS20) | (1 <<CS21) | (1<<CS22); 
  
  //set off Timer1 COMPA ISR when TCNT1 = OCR1A
  OCR1A = 1; //set to the lowest number to immediately go into ISR
  //set off Timer2 COMPB ISR when TCNT1 = OCR1B
  OCR1B = 0xFFFF; 
  //set off Timer2 COMPA ISR when TCNT2 = OCR2A
  OCR2A = 1;;
  //set off Timer2 COMPB ISR when TCNT2 = OCR2B
  OCR2B = 0xFF;

  //enable int0 interrupt
  EIMSK |= (1<<INT0);
  //set interrupt on rising edges in the EICRA 
  EICRA |= (1<<ISC01)|(1<<ISC00);
  
  //set overflow interrupt for Timer2
  TIMSK2 |= (1<<TOIE2);
  
  //enable Timer1 compare interrupt A and B
  TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B);
  //enable Timer2 compare interrupt A and B
  TIMSK2 |= (1<<OCIE2A) | (1<<OCIE2B);

  //enable global interrupts
  sei();
  
}

void loop()
{
  delay(10);
}

//ISR when button is pressed
ISR(INT0_vect){
  //breach only happens when the red LED is on
  if (colour == 1) {  
    //Set LB3 to 1 indicating there has been breached
  	LB3 = 1;
    
    //count the number of breaches
    if (breachCount > 100) {
      //once the number of breaches is above 100, it remains at 100
      breachCount = 100;
    } else {
      //increment the total number of breaches
 	  breachCount += 1;
    }
 	//immediately jump to Timer2 interrupt B
    OCR2B = 1;
  }
} 

//ISR to control the oscilloscope: set to high
ISR(TIMER1_COMPA_vect){
  //if breach is above 99 the duty cycle is 100%
  if (breachCount > 99) {
    //write HIGH to Pin 13
    PORTB |= (1<<PORTB5);
    
  //oscilloscope is only written to if a breach happens
  } else if (breachCount > 0) {
    //write HIGH to Pin 13
    PORTB |= (1<<PORTB5);
    
    //oscilloscope is set to HIGH for 1*(breachCount/100) seconds
    OCR1B = 15624 * (breachCount/100);
    
    //OCR1A is set high so OCR1B is entered first
    OCR1A = 0xFFFF;
    
    TCNT1 = 0;
  }
}

//ISR to control the oscilloscope: set to low
ISR(TIMER1_COMPB_vect){
  //oscilloscope is only written to if a breach happens
  //only written LOW if the breach count is below 100
  if ((breachCount < 100) && (breachCount > 0)) {
    //write LOW to Pin 13
    PORTB &= ~(1<<PORTB5);
    
    //oscilloscope is set to LOW for 1*(1-breachCount/100) seconds
    OCR1A = 15624* (1-breachCount/100);
    
    //OCR1B is set high so OCR1A is entered first
    OCR1B = 0xFFFF;
    
    TCNT1 = 0;
  }
}

//ISR to control the traffic lights
ISR(TIMER2_COMPA_vect) {  
  
  //when flag = 1, turn on a LED light
  if (flag == 1) {
    
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
    
    //set OCR2A to its highest value
   	OCR2A = 0xFF;
    //reset overflow
    overflow1 = 0;
    //enter part of the code that turns off the LEDs
    flag = 0;
    
  //timer count for 1s = 15624
  //15624/256(max value of 8 bit Timer2) = 61
  //Timer2 needs to overflow roughly 61 times for 1s to pass
    
  //when flag = 0, turn off a LED light
  } else if (overflow1 > 60){
    
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
    //set OCR2A to lowest value to turn on the next LED instantly
    OCR2A = 1;
    //enter part of the code that turns on the LEDs
    flag = 1;
  }
}


ISR(TIMER2_COMPB_vect) { 
  //period of 0.5s with a 50% duty cycle: on for 0.25s and off for 0.25s
  //timer count for 0.25s = 0.25*16*10^6/1024 - 1 = 3905
  //3905/256(max value of 8 bit Timer2) = 15
  //Timer2 needs to overflow roughly 15 times for 0.25s to pass
  
  //blue LED will only flash if the button is pressed when red LED is on
  if ((LB3 == 1) && (overflow2 > 14)) {
    //2 pulses will be 4 blinks 
    if (blink < 4) {
      if (flagBlue == 1) {
        //turn on the LED
        PORTC |= (1<<PC4);
        flagBlue = 0;
      } else {
        //turn off the LED
        PORTC &= ~(1<<PC4);
        flagBlue = 1;
      }
      //everytime the LED turns on or off, increment blink 
      blink += 1;
      //reset overflow
      overflow2 = 0;
      
    //once max blinks is reached reset LB3 for the next button press
    } else {
      LB3 = 0;
      //reset blinks for next button press
      blink = 0;
    }
  }
}


ISR (TIMER2_OVF_vect) {
  //increment overflow counter everytime Timer2 overflows
  overflow1 += 1;
  overflow2 += 1;
}