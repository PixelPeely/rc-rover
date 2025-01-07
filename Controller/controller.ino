#include <SPI.h>
#include "RF24.h"
//Solder breakout board to main power supply
#define PIN_CS_RADIO 10
#define PIN_CSN_RADIO A1



#define SHIFT_SER A6
#define SHIFT_SRCLK A3
#define SHIFT_RCLK A2
const uint8_t digit_buf[] = {
  0b11111100,
  0b01100000,
  0b11011010,
  0b11110010,
  0b01100110,
  0b10110110,
  0b10111110,
  0b11100000,
  0b11111110,
  0b11110110,
  0b00000001,
  0b11101100,//n
  0b10011100,//c
};

#define BATTERY_SENSE A0
#define FULL_ANALOG 1000
#define DEPLETED_ANALOG 500
#define DISPLAY_SENSITIVITY 0.001

const byte NRF24_RADIO_ADDRESS[][6] = {"00001", "00002"};
RF24 NRF24_RADIO(PIN_CS_RADIO, PIN_CSN_RADIO);
#define PAYLOAD_INPUT 0x00
#define PAYLOAD_DIAGNOSE 0x01

inline void init_NRF24_RADIO() {
  Serial.println("Initializing NRF24_RADIO");
  Serial.println("-Begin success flag: " + String(NRF24_RADIO.begin()));
  delay(100);
  NRF24_RADIO.setPALevel(RF24_PA_LOW);
  NRF24_RADIO.setPayloadSize(sizeof(uint8_t));
  NRF24_RADIO.enableAckPayload();
  NRF24_RADIO.openWritingPipe(NRF24_RADIO_ADDRESS[0]);
  NRF24_RADIO.openReadingPipe(1, NRF24_RADIO_ADDRESS[1]);
  NRF24_RADIO.stopListening();
  Serial.println("-Set config");
}

inline void init_input() {
  for (uint8_t i = 0; i < 8; i++)
    pinMode(i+2, INPUT_PULLUP);
}

inline void init_reg() {
  pinMode(SHIFT_SER, OUTPUT);
  pinMode(SHIFT_SRCLK, OUTPUT);
  pinMode(SHIFT_RCLK, OUTPUT);
}

bool send_input() {
  uint8_t input = ~(digitalRead(2) << 7 | digitalRead(3) << 6 | digitalRead(4) << 5 | digitalRead(5) << 4 | digitalRead(6) << 3 | digitalRead(7) << 2 | digitalRead(8) << 1 | digitalRead(9));
  //Serial.println("Transmitting " + String(input));
  NRF24_RADIO.flush_tx();
  return NRF24_RADIO.write(&input, sizeof(uint8_t));
}

void setup() {
  Serial.begin(115200);
  Serial.println("Begin controller 115200");
  init_NRF24_RADIO();
  init_input();
  init_reg();
  analogReference(INTERNAL1V1);
  Serial.println("Setup complete");
}

uint8_t rover_battery = 0xFF;
uint8_t controller_battery = 0xFF;
float intermediate = 0xFFFF;
void loop() {
  delay(50);
  if (send_input() && NRF24_RADIO.available()) {
    NRF24_RADIO.read(&rover_battery, sizeof(uint8_t));
    NRF24_RADIO.flush_rx();
  } else rover_battery = 0xFF;

  float analog_reading = ((float)analogRead(BATTERY_SENSE) - DEPLETED_ANALOG) / (FULL_ANALOG - DEPLETED_ANALOG) * 100;
  if (intermediate == 0xFFFF) intermediate = analog_reading;
  intermediate += (analog_reading - intermediate) * DISPLAY_SENSITIVITY;
  if (intermediate < 0) intermediate = 0;
  else if (intermediate > 100) intermediate = 100;
  if (intermediate > controller_battery + 2) controller_battery = (uint8_t)intermediate;
  if (intermediate < controller_battery) controller_battery = (uint8_t)intermediate;
  if (abs(analog_reading - controller_battery) > 25) controller_battery = intermediate = analog_reading;
  update_battery_display(controller_battery, rover_battery);
}

void update_battery_display(uint8_t controller_battery, uint8_t rover_battery) {
  uint8_t controller_digit1 = controller_battery / 10;
  uint8_t controller_digit2 = controller_battery - controller_digit1 * 10;
  uint8_t rover_digit1 = rover_battery / 10;
  uint8_t rover_digit2 = rover_battery - rover_digit1 * 10;

  if (rover_battery == 0xFF) {
    rover_digit1 = 11;
    rover_digit2 = 12;
  }

  digitalWrite(SHIFT_RCLK, LOW);
  shiftOut(SHIFT_SER, SHIFT_SRCLK, LSBFIRST, ~digit_buf[controller_digit2]);
  shiftOut(SHIFT_SER, SHIFT_SRCLK, LSBFIRST, ~digit_buf[controller_digit1]);
  shiftOut(SHIFT_SER, SHIFT_SRCLK, LSBFIRST, ~digit_buf[rover_digit1]);
  shiftOut(SHIFT_SER, SHIFT_SRCLK, LSBFIRST, ~digit_buf[rover_digit2]);
  digitalWrite(SHIFT_RCLK, HIGH);
}