// Based on http://ava.upuaut.net/?p=408

#define ASCII 7          // ASCII 7 or 8
#define STOPBITS 2       // Either 1 or 2
#define TXDELAY 0        // Delay between sentence TX's
#define RTTY_BAUD 50    // Baud rate for use with RFM22B Max = 600

#define RADIOPIN 0

volatile int txstatus=1;
volatile int txstringlength=0;
volatile char txc;
volatile int txi;
volatile int txj;

void initialise_radio_interrupt(void);
void rtty_txbit (int bit);

/*
   Interrupt Initialization.
   HARDWARE:
   VARIABLE:
*/
void initialise_radio_interrupt()
{
  pinMode(RADIOPIN, OUTPUT);
  // initialize Timer2
  cli();          // disable global interrupts
  TCCR2A = 0;     // set entire TCCR1A register to 0
  TCCR2B = 0;     // same for TCCR1B
  OCR2A = F_CPU / 1024 / RTTY_BAUD - 1;  // set compare match register to desired timer count:
  TCCR2A |= (1 << WGM21);   // turn on CTC mode:
  // Set CS20, CS21 and CS22 bits for:
  TCCR2B |= (1 << CS20);
  TCCR2B |= (1 << CS21);
  TCCR2B |= (1 << CS22);
  // enable timer compare interrupt:
  TIMSK2 |= (1 << OCIE2A);
  sei();          // enable global interrupts
}

/*
   NTX2B Interrupt Routine.
   HARDWARE: NTX2B
   VARIABLE:
*/
ISR(TIMER2_COMPA_vect)
{
  switch (txstatus) {
    case 0: // This is the optional delay between transmissions.
      txj++;
      if (txj > (TXDELAY * RTTY_BAUD)) {
        txj = 0;
        txstatus = 1;
      }
      break;
    case 1: // Initialise transmission, take a copy of the string so it doesn't change mid transmission.
      strcpy(txstring,datastring);
      txstringlength = strlen(txstring);
      txstatus = 2;
      txj = 0;
      break;
    case 2: // Grab a char and lets go transmit it.
      if ( txj < txstringlength)
      {
        txc = txstring[txj];
        txj++;
        txstatus = 3;
        rtty_txbit (0); // Start Bit;
        txi = 0;
      }
      else
      {
        txstatus = 0; // Should be finished
        txj = 0;
      }
      break;
    case 3:
      if (txi < ASCII)
      {
        txi++;
        if (txc & 1) rtty_txbit(1);
        else rtty_txbit(0);
        txc = txc >> 1;
        break;
      }
      else
      {
        rtty_txbit (1); // Stop Bit
        txstatus = 4;
        txi = 0;
        break;
      }
    case 4:
      if (STOPBITS == 2)
      {
        rtty_txbit (1); // Stop Bit
        txstatus = 2;
        break;
      }
      else
      {
        txstatus = 2;
        break;
      }

  }
}

/*
   NTX2B Driving Routine.
   HARDWARE: NTX2B
   VARIABLE:
*/
void rtty_txbit (int bit)
{
  if (bit)
  {
    digitalWrite(RADIOPIN, HIGH); // High
  }
  else
  {
    digitalWrite(RADIOPIN, LOW); // Low
  }
}
