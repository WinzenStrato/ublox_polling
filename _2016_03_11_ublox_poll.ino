#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>

char datastring[80];
char txstring[80];
String databuffer_gps_received;
char databuffer_gps[120];
char gps_process[120];
unsigned int count = 0;
byte XOR;
byte gps_set_sucess = 0;

void setup() {
  // put your setup code here, to run once:
  init_UART();
  initialise_radio_interrupt();
  setup_gps();
}

void loop() {
  get_data_gps();
  prep_data();
  Serial1.print(validateChecksum(databuffer_gps));
  Serial1.print(F(","));
  Serial1.println(databuffer_gps);
}

void init_UART() {
  Serial.begin(9600);
  Serial1.begin(19200);
  delay(100);
}

void prep_data() {
  sprintf(datastring, "$$$$$M0UPU,%04u,RTTY TEST BEACON RTTY TEST BEACON - PHYSICS OSCILLATES", count); // Puts the text in the datastring
  unsigned int CHECKSUM = gps_CRC16_checksum(datastring);  // Calculates the checksum for this datastring
  char checksum_str[6];
  sprintf(checksum_str, "*%04X\n", CHECKSUM);
  strcat(datastring, checksum_str);
  count++;
}

uint16_t gps_CRC16_checksum (char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;

  crc = 0xFFFF;

  // Calculate checksum ignoring the first two $s
  for (i = 5; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }

  return crc;
}
