/*
Task 4:
Implement a basic traffic-light system. The traffic light changes state every second. One
cycle is as follows: red to green, green to yellow, yellow to red.
*/

//variable to keep track of the colour 1 = red, 2 = green, 3 = orange
volatile uint8_t colour = 1;

void setup()
{
  //disable global interrupts
  cli();

  DDRB |= (1 << DDB2); //set Pin 10 as output (red)
  DDRB |= (1 << DDB1); //set Pin 9 as output (yellow)
  DDRB |= (1 << DDB0); //set Pin 8 as output (green)
  
  //initialize counter and control registers on normal mode  
  TCCR1A = 0;  
  TCCR1B = 0;
  TCNT1 = 0; 
  
  //set to a low value to instantly enter interrupt
  OCR1A = 1;
  //set to 1 second: (1 * 16 * 10^6 / 1024) - 1  = 15624
  OCR1B = 15624;

  //setting prescaler to 1024 for Timer1
  TCCR1B |= (1<<CS10) | (0 <<CS11) | (1<<CS12); 

  //enable Timer1 compare interrupt A and B
  TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B);
  
  //enable global interrupts
  sei();
}

void loop()
{
  asm volatile("nop");
}

ISR(TIMER1_COMPA_vect) { 
  
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
 
} 

ISR(TIMER1_COMPB_vect) { 
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
  TCNT1 = 0;
}
