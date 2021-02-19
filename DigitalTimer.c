/*
 *  DigitalTimer.c
 *      Author: Shams Elgohary
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define STOP_TIMER		 TCCR1B &= ~(1<< CS10) & ~(1<<CS12) // CLEARS PRESCALE BITS SO COUNTER STOPS
#define RESUME_TIMER 	 TCCR1B |= (1<<CS10) | (1<<CS12)   // SETS PRESCALE BITS SO COUNTER RESUMES
#define DELAY_TIME 1000
#define SET_BIT(REG,BIT_NUM) 		REG = ( REG | (1<<BIT_NUM));
#define RESET_BIT(REG,BIT_NUM) 		REG = ( REG & ~(1<<BIT_NUM));

unsigned char SECOND_1=0, SECOND_2=0, MINUTE_1 =0, MINUTE_2 =0, HOUR_1 =0, HOUR_2=0;

/*  ***********************   INTERRUPT SERVICE ROUTINE   ********************************************* */

ISR (INT0_vect){
	SECOND_1=0, SECOND_2=0, MINUTE_1 =0, MINUTE_2 =0, HOUR_1 =0, HOUR_2=0;
}

ISR(INT1_vect)	 		 {		STOP_TIMER;   	}

ISR(INT2_vect)  		 { 		RESUME_TIMER; 	}

ISR (TIMER1_COMPA_vect)  { 	    SECOND_1 ++;    }

/*  ********************************************************************************************* */

void INT0_init() 						// RESET TIMER INTERRUPT //
{
	DDRD &= ~(1<<PD2); // INPUT
	PORTD |= (1<<PD2); // Enabling the internal pull up resistor

	MCUCR |= (1<<ISC01); //Falling edge of INT0 generates interrupt request ISC01 =1 ISC00 =0
	GICR |= (1<<INT0); // External Interrupt Request
	SREG |= (1<<7); // Enabling i-bit
}

void INT1_init()						// PAUSE TIMER INTERRUPT //
{
	DDRD &= ~ (1<<PD3); //INPUT

	MCUCR |= (1<<ISC11) | (1<<ISC10); // Rising edge of INT1 generates interrupt request ISC11=1 ISC10 =1
	GICR |= (1<<INT1); // External Interrupt Request
	SREG |= (1<<7); // Enabling i-bit

}

void INT2_init()  						// RESUME TIMER INTERRUPT //
{
	DDRB &= ~ (1<<PB2); //INPUT
	PORTB |= (1<<PB2); // Enabling the internal pull up resistor

	MCUCSR &= ~(1<<ISC2); // Falling Edge of INT2 by Default = 0
	GICR |= (1<<INT2); // External Interrupt Request
	SREG |= (1<<7); // Enabling i-bit

}

void Timer1_CTC_init()
{
	/*
	 * F_CPU = 1 MHZ  F_TIMER = 1 uS
	 * PREFACTOR = 1024 (CS10 =1 CS12 =1)  T_TIMER = 1 MS approximately
	 * FOC1A = 1 NON-PWM MODE
	 * WGM12=1 MODE 4 CTC
	 */

	TCCR1A = (1<<FOC1A);
	TCCR1B = (1<<WGM12) | (1<< CS10) | (1<<CS12) ;

	TIMSK = (1<<OCIE1A);  // Compare Match Interrupt Enable

	TCNT1 = 0;
	OCR1A = 976; // OCR1A WOULD BE EQUAL TO 1000 BUT T_TIMER ISN'T EXACTLY 1 MS

}

/*  ***************************************  MAIN  *********************************************** */


int main(void)
{
	INT0_init();
	INT1_init();
	INT2_init();
	Timer1_CTC_init();

	DDRC |= 0X0F;  // First 4 bits = 1 (OUTPUTS)
	PORTC &= 0xF0; //Default = 0 ;

	DDRA |= 0X3F; // FIRST 6 BITS = 1 (OUTPUTS)

	while(1)
	{

	while(SECOND_1 < 10) // While loop to prevent checking if conditions each cycle ( save time )
   {
		SET_BIT(PORTA,0); 									// SECOND 1 DISPLAY ON
		PORTC = (PORTC & 0XF0) | (SECOND_1 & 0x0F);			// PORTC = SECONDS DIGIT
		_delay_us(DELAY_TIME);
		RESET_BIT(PORTA,0);


		SET_BIT(PORTA,1); 									//SECOND 2 DISPLAY ON
		PORTC = (PORTC & 0XF0) | (SECOND_2 & 0x0F);			// PORTC = SECONDS TENTH DIGIT
		_delay_us(DELAY_TIME);
		RESET_BIT(PORTA,1);

		SET_BIT(PORTA,2);
		PORTC = (PORTC & 0XF0) | (MINUTE_1 & 0x0F);
		_delay_us(DELAY_TIME);
		RESET_BIT(PORTA,2);

    	SET_BIT(PORTA,3);
		PORTC = (PORTC & 0XF0) | (MINUTE_2 & 0x0F);
		_delay_us(DELAY_TIME);
		RESET_BIT(PORTA,3);

		SET_BIT(PORTA,4);
		PORTC = (PORTC & 0XF0) | (HOUR_1 & 0x0F);
		_delay_us(DELAY_TIME);
		RESET_BIT(PORTA,4);

		SET_BIT(PORTA,5);
		PORTC = (PORTC & 0XF0) | (HOUR_2 & 0x0F);
		_delay_us(DELAY_TIME);
		RESET_BIT(PORTA,5);

		}

		  SECOND_2++; SECOND_1=0;			// TENTH SECOND DIGIT

	if (SECOND_2 == 6)						// 60 SECONDS = 1 MINUTE
		{ MINUTE_1++;  SECOND_2=0;  }

	if (MINUTE_1 == 10)						// TENTH MINUTE DIGIT
		{ MINUTE_2 ++; MINUTE_1=0;	}

	if (MINUTE_2 == 6)						// 60 MINUTES = 1 HOUR
		{ HOUR_1 ++;   MINUTE_2=0;	}

	if ( HOUR_1 == 10)						// TENTH HOUR DIGIT
		{ HOUR_2++;    HOUR_1=0;    }

	if ((HOUR_2 == 2) && (HOUR_1 == 4)) // IF 24 HOURS PASS START A NEW DAY 00 : 00 : 00
		{ SECOND_1=0, SECOND_2=0, MINUTE_1 =0, MINUTE_2 =0, HOUR_1 =0, HOUR_2=0; }
	}
}
