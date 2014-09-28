#include <msp430g2553.h>
#include <stdlib.h>
#include <string.h>

char RXbuffer[20], *mode, *p;
unsigned int i=0, doneFlag = 0, r, g, b, bit, val;

void UART_TX(char * tx_data);          			// Function Prototype for TX

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;					//Stop WDT
//	BCSCTL1 = CALBC1_1MHZ;						//Set DCO to 1MHz
//	DCOCTL = CALDCO_1MHZ;
	BCSCTL1 = CALBC1_12MHZ;						//Set DCO to 1MHz
	DCOCTL = CALDCO_12MHZ;


	P1DIR |= (BIT0 | BIT6);						//Set P1.0 and P1.6 to outputs
	P1OUT &= ~(BIT0+BIT3+BIT4+BIT5+BIT6+BIT7);	//Turn all outputs off
	P1SEL |= BIT1+BIT2;							//Set P1.1 and P1.2 to RX/TX
	P1SEL2|= BIT1+BIT2;

	//rgb led stuff
    P2OUT = 0;									//Turn off all Port 2 outputs
    P2DIR |= BIT2+BIT4+BIT6;                    //Those are the selected outputs
    P2SEL |= BIT2+BIT4+BIT6;                    //Select the timers on those pins
    P2SEL &= ~BIT7;								//Disable XOUT or TA on 2.6 won't work :<

	//UART setup
	UCA0CTL1 |= UCSSEL_2;						//SMCLK
//	UCA0BR0 = 109;								//9600bps	//1040000Hz/9600
//	UCA0BR1 = 0;								//9600bps
    UCA0BR0 = 113;
    UCA0BR1 = 2;

	UCA0MCTL = UCBRS_1;							//Modulation
	UCA0CTL1 &= ~UCSWRST;						//Start USCI
	IE2 |= UCA0RXIE;							//Enable RX interrupt
	__enable_interrupt();						//Enable global interrupts

	//P2.6 PWM setup
    TA0CTL = TASSEL_2 + MC_1;                  	// SMCLK, upmode
    //TA0CCTL1 = OUTMOD_3;                      // TA0CCTL1 - blue PWM set/reset FOR RGB LED
    TA0CCTL1 = OUTMOD_7;                      	// TA0CCTL1 - blue PWM reset/set FOR RGB STRIP
    TA0CCR0 = 255;                             	// PWM Period

    //P2.2 + P2.4 PWM setup
    TA1CTL = TASSEL_2 + MC_1;        	        // SMCLK, upmode
    //TA1CCTL1 = OUTMOD_3;                      // TA1CCTL1 - red PWM set/reset FOR RGB LED
    TA1CCTL1 = OUTMOD_7;    	                // TA1CCTL1 - red PWM reset/set FOR STRIP

    //TA1CCTL2 = OUTMOD_3;                      // TA1CCTL2 - green PWM set/reset FOR RGB LED
    TA1CCTL2 = OUTMOD_7;						// TA1CCTL2 - green PWM reset/set FOR STRIP
    TA1CCR0 = 255;          	                // PWM Period

}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){
	
	if(doneFlag == 1)
	{
		i = 0;
		doneFlag = 0;
		mode = "";
	}

	RXbuffer[i] = UCA0RXBUF;
	i++;

	if(UCA0RXBUF == 59) // ; = end of packet character
		doneFlag = 1;

	if(doneFlag == 1) //tokenize received string
		mode = strtok (RXbuffer," ;");

	if(doneFlag)
	{
		if(strcmp(mode, "off") == 0) //off ;
		{
			P1OUT &= ~BIT0;
			P1OUT &= ~BIT6;
			TA1CCR1 = 0;
			TA1CCR2 = 0;
			TA0CCR1 = 0; //...turn off the lights
			UART_TX("\n\r>Idling...\n\r");
		}
		else if(strcmp(mode, "rst") == 0)
		{
			UART_TX("\n\r>Rebooting...\n\r");
			__delay_cycles(1048);
			WDTCTL = 0;
		}
		else if(strcmp(mode, "ping") == 0)
		{
			UART_TX("\n\r>pong!\n\r");
		}
		else if(strcmp(mode, "rgb") == 0) //rgb x,x,x; rgb values 0-255
		{
			p = RXbuffer+4;		//point to the first number (NL+ mode code)
			r = atoi(p);

			while(*p != ',')
				p++;
			p++;
			g = atoi(p);

			while(*p != ',')
				p++;
			p++;
			b = atoi(p);

		if(r >= 0 && r <=255 && g >= 0 && g <=255 && b >= 0 && b <=255)
		{
			TA1CCR1 = r;
			TA1CCR2 = g;
			TA0CCR1 = b;
		}
		else
			UART_TX("\n\rAcceptable rgb values: 0 - 255.\n\r");
		}
		else if(strcmp(mode, "bit") == 0) //bit 1,1;	= BIT1 HI (P1 only)
		{
			p = RXbuffer + 4;		//point to the first number (NL+ mode code)
			bit = atoi(p);

			while(*p != ',')
				p++;
			p++;
			val = atoi(p);

			if(bit == 0)
			{
				if(val == 1)
					P1OUT |= BIT0;
				else if(val == 0)
					P1OUT &= ~BIT0;
			}
			else if(bit == 6)
			{
				if(val == 1)
					P1OUT |= BIT6;
				else if(val == 0)
					P1OUT &= ~BIT6;
			}
		}
		else if(strcmp(mode, "sta") == 0) //sta ; P1OUT and P2OUT bit status
		{
			UCA0TXBUF = P1OUT;
			__delay_cycles(1048);
			UCA0TXBUF = P2OUT;
		}
		else if(strcmp(mode, "help") == 0 || strcmp(mode, "man") == 0) // help ; or man ;
		{
			UART_TX("\n\r>Hello and welcome to the SERIAL KILLER 3000 help menu!\n\n\r");
			UART_TX("\n\r>Available commands:");
			UART_TX("\n\r>off;\t\tIdle mode, all outputs go LOW, PWM to 0% duty cycle.");
			UART_TX("\n\r>rst;\t\tSoft reset.");
			UART_TX("\n\r>ping;\t\tThe SERIAL KILLER 3000 replies accordingly.");
			UART_TX("\n\r>rgb r,g,b;\tRGB mode, acceptable values 0 - 255. \n\r\t\tMax load per chan: 3.5A(cont)/15A(pulsed).");
			UART_TX("\n\r>bit x,y;\tPort 1 GPIO control. x = pin number(0 - 7) y = pin state(0 - 1)\n\r\t\t[only bits 0 and 6 available on the Launchpad].");
			UART_TX("\n\r>sta;\t\tReturns Port 1 and Port 2 GPIO status (GUI only).");
			UART_TX("\n\r>man;/help ;\tPrints this screen.\n\n\r");
		}
		else
		{
			mode = NULL;
			for(i=0; i<20; i++)
				RXbuffer[i] = NULL;

			UART_TX("\n\r>Unknown command.");
			UART_TX("\n\r>To see available commands enter help;\n\n\r");
		}
	}
}

void UART_TX(char * tx_data) // Define a function which accepts a character pointer to an array
{
	unsigned int f = 0;
    while(tx_data[f]) // Increment through array, look for null pointer (0) at end of string
    {
        while ((UCA0STAT & UCBUSY)); // Wait if line TX/RX module is busy with data
        UCA0TXBUF = tx_data[f]; // Send out element i of tx_data array on UART bus
        f++; // Increment variable for array address
    }
}
