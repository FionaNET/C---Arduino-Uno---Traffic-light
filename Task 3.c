/*
Task 3:
Implement a system that estimates the speed of vehicles as they approach the freeway.
The speed of vehicles is estimated using the light barriers LB1 and LB2.
*/

//variable setup:
//contains the duty cycle - beginning has a duty cycle of 0%
volatile long int dutyCycle = 0;
//contains the count when LB2 is passed
volatile float LB2Time = 0;
//time passed between LB1 and LB2 in seconds
volatile float time = 0;
//counts the number of times Timer 1 overflows
volatile float overflow = 0;

void setup()
{
  //disables interrupts
  cli();
  
  //set pin 9 to be output
  DDRB |= (1<<DDB1);
  //set pin 2 and 3 to be input
  DDRD &= ~(1<<DDD2); //LB1
  DDRD &= ~(1<<DDD3); //LB2
    
  //enable external interrupts in the EIMSK (INT0, INT1)
  EIMSK |= (1<<INT0)|(1<<INT1);
  //set interrupt on rising edges in the EICRA 
  //(ISC01, ISC00, ISC11, ISC10 all set to 1)
  EICRA |= (1<<ISC01)|(1<<ISC00)|(1<<ISC10)|(1<<ISC11);
  
  TCCR1A = 0;  
  TCCR1B = 0;
  
  //set up Timer 1 as fast PWM (mode 14)
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << WGM13);
  
  
  //prescaler of 1024 for Timer 1
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);

  //set PWM top value 
  ICR1 = 15624; //equates to 1 second

  //set PWM duty cycle (start duty cycle is 0%)
  OCR1A = 0;

  //set overflow interrupt
  TIMSK1 |= (1<<TOIE1);
 
  //set as non-inverting compare output mode
  TCCR1A |= (1 << COM1A1);
  
  //enable interrupts
  sei();
}


void loop()
{
  asm volatile("nop");  
}

//Interrupt for LB1 - tactile push button 1
ISR(INT0_vect){
  //set timer 1 to zero to start counting
  TCNT1 = 0;
  //set overflow to zero to start counting
  overflow = 0;
}

//Interrupt for LB2 - tactile push button 2
ISR(INT1_vect){
  //save the count passed between LB1 and LB2
  LB2Time = TCNT1;
  
  //multiply counter by 1024/16*10^6 to convert to seconds
  time = (LB2Time + ICR1*overflow) * 0.000064;

  //if time is equal to or below 0.72, speed >= 100 and hence duty cycle = 100%
  if (time <= 0.72) {
    dutyCycle = 100;
    
  } else {
    //speed(km/hr) = Duty Cycle % = 72/time
  	dutyCycle = 72/time;
  }
  
  //convert OCR1A to display the new duty cycle
  OCR1A = (ICR1*dutyCycle)/100;
}

ISR (TIMER1_OVF_vect) {
  //reset Timer1
  TCNT1 = 0;
  //count everytime Timer1 overflows
  overflow += 1;
}
