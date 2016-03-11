void setup_gps(void);
void poll_gps_data(void);
void convert_gps_data(void);
void get_data_gps(void);

void setup_gps() {
  Serial1.println(F("GPS Startup"));
  //Automatischen NMEA Output deaktivieren
  Serial.println("$PUBX,40,GLL,0,0,0,0*5C");
  Serial.println("$PUBX,40,GGA,0,0,0,0*5A");
  Serial.println("$PUBX,40,GSA,0,0,0,0*4E");
  Serial.println("$PUBX,40,RMC,0,0,0,0*47");
  Serial.println("$PUBX,40,GSV,0,0,0,0*59");
  Serial.println("$PUBX,40,VTG,0,0,0,0*5E");
  delay(3000);
  // Set the GPS to airborne mode
  uint8_t setNav[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
  };
  while (!gps_set_sucess)
  {
    sendUBX(setNav, sizeof(setNav) / sizeof(uint8_t));
    gps_set_sucess = getUBX_ACK(setNav);
  }
  gps_set_sucess = 0;
  Serial1.println(F("GPS Startup Complete"));
}

void get_data_gps() {
  poll_gps_data();
  convert_gps_data();
}

void poll_gps_data(void) {
  Serial.println("$PUBX,00*33"); //poll request for stadard output
  while (Serial.available() == 0) {}
  databuffer_gps_received = Serial.readStringUntil('\n');
}

void convert_gps_data() {
  databuffer_gps_received.toCharArray(databuffer_gps, 120);
  strcpy(gps_process, databuffer_gps);
  char *initial = strtok(gps_process, ",");
  char *id  = strtok(NULL, ",");
  char *time_gps = strtok(NULL, ",");
  char *latitude = strtok(NULL, ",");
  char *latitude_letter = strtok(NULL, ",");
  char *longitude = strtok(NULL, ",");
  char *longitude_letter = strtok(NULL, ",");
  char *altitude_ = strtok(NULL, ",");
  char *navstat = strtok(NULL, ",");
  char *HACC = strtok(NULL, ",");
  char *VACC = strtok(NULL, ",");
  char *Speed = strtok(NULL, ",");
  char *Course = strtok(NULL, ",");
  char *Vario = strtok(NULL, ",");
  char *RX_age = strtok(NULL, ",");
  char *HDOP = strtok(NULL, ",");
  char *VDOP = strtok(NULL, ",");
  char *TDOP = strtok(NULL, ",");
  char *number_GPS = strtok(NULL, ",");
  char *number_GLONASS = strtok(NULL, ",");
  char *DR = strtok(NULL, ",");

  /*  long gps_time = atoi(time_gps);
    byte gps_hour_high = gps_time / 100000;
    byte gps_hour_low = gps_time / 10000;
    byte gps_min_high = gps_time / 1000;
    byte gps_min_low = gps_time / 100;
    byte gps_sec_high = gps_time / 10;
    byte gps_sec_low = gps_time / 1;
    gps_hour = word(gps_hour_high, gps_hour_low);
     gps_min = word(gps_min_high, gps_min_low);
      gps_sec = word(gps_sec_high, gps_sec_low);
  */
}

// Calculates the checksum for a given string
// returns as integer
int getCheckSum(char *string) {
  int i;
  int XOR;
  int c;
  // Calculate checksum ignoring any $'s in the string
  for (XOR = 0, i = 0; i < strlen(string); i++) {
    c = (unsigned char)string[i];
    if (c == '*') break;
    if (c != '$') XOR ^= c;
  }
  return XOR;
}

// Validates the checksum on an (for instance NMEA) string
// Returns 1 on valid checksum, 0 otherwise
int validateChecksum(char *string) {
  char *gotSum = strchr(string, '*');
  char CK_A = gotSum[1];
  char CK_B = gotSum[2];
  // Check that the checksums match up
  if ((16 * atoh(CK_A)) + atoh(CK_B) == getCheckSum(databuffer_gps)) return 1;
  else return 0;
}

// Converts a HEX string to an int
int atoh(char c) {
  if (c >= 'A' && c <= 'F')
    return c - 55;
  else if (c >= 'a' && c <= 'f')
    return c - 87;
  else
    return c - 48;
}

// Send a byte array of UBX protocol to GPS
void sendUBX(uint8_t *MSG, uint8_t len) {
  for (int i = 0; i < len; i++) {
    Serial.write(MSG[i]);
  }
}

// Calculate expected UBX ACK packet and parse UBX response from GPS
boolean getUBX_ACK(uint8_t *MSG) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  Serial1.print(" * Reading ACK response: ");

  // Construct the expected ACK packet
  ackPacket[0] = 0xB5;  // header
  ackPacket[1] = 0x62;  // header
  ackPacket[2] = 0x05;  // class
  ackPacket[3] = 0x01;  // id
  ackPacket[4] = 0x02;  // length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2];  // ACK class
  ackPacket[7] = MSG[3];  // ACK id
  ackPacket[8] = 0;   // CK_A
  ackPacket[9] = 0;   // CK_B

  // Calculate the checksums
  for (uint8_t i = 2; i < 8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }

  while (1) {

    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      Serial1.println(" (SUCCESS!)");
      return true;
    }

    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) {
      Serial1.println(" (FAILED!)");
      return false;
    }

    // Make sure data is available to read
    if (Serial.available()) {
      b = Serial.read();

      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) {
        ackByteID++;
        Serial1.print(b, HEX);
      }
      else {
        ackByteID = 0;  // Reset and look again, invalid order
      }

    }
  }
}
