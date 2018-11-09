/*
  NAME:
  gbj_twowire

  DESCRIPTION:
  Library embraces and provides common methods used at every application
  working with sensor on two-wire (I2C) bus.
  - Library specifies (inherits from) the system TwoWire library.
  - Library implements extended error handling.
  - Library provides some general system methods implemented differently for
    various platforms, especially Arduino, ESP8266, ESP32, and Particle.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the license GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
  (related to original code) and MIT License (MIT) for added code.

  CREDENTIALS:
  Author: Libor Gabaj
  GitHub: https://github.com/mrkaleArduinoLib/gbj_twowire.git
 */
#ifndef GBJ_TWOWIRE_H
#define GBJ_TWOWIRE_H

#if defined(__AVR__)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
  #include <inttypes.h>
  #include <Wire.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <Arduino.h>
  #include <Wire.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#endif


class gbj_twowire : public TwoWire
{
public:
//------------------------------------------------------------------------------
// Public constants
//------------------------------------------------------------------------------
static const String VERSION;
enum ResultCodes
{
  SUCCESS = 0,
  #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  // Arduino, ESP error codes
  ERROR_BUFFER = 1,  // Data too long to fit in transmit buffer
  ERROR_NACK_ADDR = 2,  // Received NACK on transmit of address
  ERROR_NACK_DATA = 3,  // Received NACK on transmit of data
  ERROR_NACK_OTHER = 4,  // Other error
  // Arduino custom errors
  ERROR_ADDRESS = ERROR_NACK_ADDR,
  #elif defined(PARTICLE)
  // Particle error codes
  ERROR_BUSY = 1,  // Busy timeout upon entering endTransmission()
  ERROR_START = 2,  // START bit generation timeout
  ERROR_END = 3,  // End of address transmission timeout
  ERROR_TRANSFER = 4,  // Data byte transfer timeout
  ERROR_TIMEOUT = 5,  // Data byte transfer succeeded, busy timeout immediately after
  // Particle custom errors
  ERROR_ADDRESS = ERROR_START,
  #endif
  ERROR_PINS = 10, // Error defining pins, usually both are the same
};
enum ClockSpeed
{
  CLOCK_100KHZ = 100000L,
  CLOCK_400KHZ = 400000L,
};


//------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------
/*
  Constructor.

  DESCRIPTION:
  Constructor creates the class instance object and sets some bus parameters.
  Those parameters can be changed individually later in a sketch, if needed
  to change them dynamically.

  PARAMETERS:
  clockSpeed - Two-wire bus clock frequency in Hertz. If the clock is not from
               enumeration, it fallbacks to 100 kHz.
               - Data type: long
               - Default value: CLOCK_100KHZ
               - Limited range: CLOCK_100KHZ, CLOCK_400KHZ

  busStop - Flag about releasing the bus after end of data transmission.
            - Data type: boolean
            - Default value: true
            - Limited range: true, false

  pinSDA - Microcontroller's pin for serial data.
          - Data type: non-negative integer
          - Default value: 4 (GPIO4, D2)
          - Limited range: 0 ~ 255

  pinSCL - Microcontroller's pin for serial clock.
          - Data type: non-negative integer
          - Default value: 5 (GPIO5, D1)
          - Limited range: 0 ~ 255

  RETURN:  object
*/
gbj_twowire(uint32_t clockSpeed = CLOCK_100KHZ, bool busStop = true, \
  uint8_t pinSDA = 4, uint8_t pinSCL = 5);


/*
  Destructor.

  DESCRIPTION:
  Destructor releases two-wire bus.

  PARAMETERS: none

  RETURN:  none
*/
~gbj_twowire();


/*
  Initialize two wire bus and check parameters stored by constructor.

  DESCRIPTION:
  The method validate pin definitions from constructor for software defined
  I2C bus microcontrollers (ESP8266, ESP32).

  PARAMETERS: none

  RETURN:
  Result code.
*/
uint8_t begin();


/*
  Release two-wire bus.

  DESCRIPTION:
  The pins used by the two-wire bus are available for general purpose I/O.

  PARAMETERS: none

  RETURN: none
*/
void release();


/*
  Send byte stream to the two-wire bus.

  DESCRIPTION:
  The method sends input data byte array to the two-wire bus chunked by parent
  library data buffer length.

  PARAMETERS:
  dataBuffer - Pointer to the byte data buffer array.
              - Data type: non-negative integer
              - Default value: none
              - Limited range: address space

  dataLen - Number of bytes to be sent from the data buffer to the bus.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  RETURN:
  Result code.
*/
uint8_t busSendStream(uint8_t* dataBuffer, uint16_t dataLen, bool dataReverse = false);

uint8_t busSendStreamPrefixed(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse, \
  uint8_t *prfxBuffer, uint16_t prfxLen, bool prfxReverse);

/*
  Send one or two bytes to the two-wire bus.

  DESCRIPTION:
  The method sends input data to the two-wire bus as one communication
  transaction.
  - The method is overloaded.
  - In case of two parameters, the first one is considered as a command and
    second one as the data. In this case the method sends 2 ~ 4 bytes to the bus
    in one transaction.
  - In case of one parameter, it is considered as the general data and in fact
    might be a command or the data. In this case the method sends 1 ~ 2 bytes
    to the bus in one transaction.
  - If the most significant byte (MSB - the first one from the left) of either
    parameter is non-zero, the data is written as two subsequent bytes with MSB
    first.
  - If the MSB of either parameter is zero, the data is written as just its
    Least significant byte (LSB).

  PARAMETERS:
  command - Word or byte to be sent in the role of command.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  data - Word or byte to be sent in the role of data.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: 0 ~ 65535

  RETURN:
  Result code.
*/
uint8_t busSend(uint16_t command, uint16_t data);
uint8_t busSend(uint16_t data);

/*
  Read one byte from the two-wire bus.

  DESCRIPTION:
  The method reads one byte from the two-wire bus in respect to the current
  platform.

  PARAMETERS: none

  RETURN:
  Data byte read from the bus.
*/
uint8_t busRead();


/*
  Read multiple bytes from the two-wire bus.

  DESCRIPTION:
  The method reads multiple bytes from the two-wire bus and places them to the
  array defined by an input pointer.

  PARAMETERS:
  dataArray - Pointer to an array of bytes for storing read data. The array
              should be enough large for storing all read bytes.
              - Data type: array of non-negative integer
              - Default value: none
              - Limited range: platform specific address space

  bytes - Number of bytes to be read.
          - Data type: non-negative integer
          - Default value: none
          - Limited range: 0 ~ 255

  start - The array index where to start storing read bytes.
          - Data type: non-negative integer
          - Default value: 0
          - Limited range: 0 ~ 255

  RETURN:
  Result code and read data bytes in the input array at success.
*/
uint8_t busReceive(uint8_t dataArray[], uint8_t bytes, uint8_t start = 0);


/*
  Send software reset command to general call address.

  DESCRIPTION:
  The method sends command 0x06 to the general call address 0x00 in order to
  execute software reset of all devices on the two-wire bus that have this
  functionality implemented.

  PARAMETERS: none

  RETURN:
  Result code.
*/
uint8_t busGeneralReset();


//------------------------------------------------------------------------------
// Public setters - they usually return result code or void.
//------------------------------------------------------------------------------
inline void initLastResult() { _busStatus.lastResult = SUCCESS; };
inline uint8_t setLastResult(uint8_t lastResult = SUCCESS) { return _busStatus.lastResult = lastResult; };
inline void setBusStop(bool busStop) { _busStatus.busStop = busStop; };
uint8_t setAddress(uint8_t address);
uint8_t setPins(uint8_t pinSDA, uint8_t pinSCL);

/*
  Set frequency of the two-wire bus.

  DESCRIPTION:
  Method updates the bus clock frequency in the class instance object only. It
  takes effect at next bus initialization by using method busSend or busReceive.

  PARAMETERS:
  clockSpeed - Two-wire bus clock frequency in Hertz. If the clock is not from
               enumeration, it fallbacks to 100 kHz.
               - Data type: long
               - Default value: CLOCK_100KHZ
               - Limited range: CLOCK_100KHZ, CLOCK_400KHZ

  RETURN: none
*/
void setBusClock(uint32_t clockSpeed);


//------------------------------------------------------------------------------
// Public getters
//------------------------------------------------------------------------------
inline uint8_t getLastResult() { return _busStatus.lastResult; }; // Result of a recent operation
inline uint8_t getLastCommand() { return _busStatus.lastCommand; }; // Command code of a recent operation
inline uint8_t getAddress() { return _busStatus.address; };  // Current device address
inline uint8_t getAddressMin() { return ADDRESS_MIN; };  // Adress limits...
inline uint8_t getAddressMax() { return ADDRESS_MAX; };
inline uint8_t getAddressMinSpecial() { return ADDRESS_MIN_SPECIAL; };
inline uint8_t getAddressMinUsual() { return ADDRESS_MIN_USUAL; };
inline uint8_t getAddressMaxUsual() { return ADDRESS_MAX_USUAL; };
inline uint8_t getPinSDA() { return _busStatus.pinSDA; };
inline uint8_t getPinSCL() { return _busStatus.pinSCL; };
inline uint32_t getBusClock() { return _busStatus.clock; };  // Bus clock frequency in Hz
inline bool isSuccess() { return _busStatus.lastResult == SUCCESS; } // Flag about successful recent operation
inline bool isError() { return !isSuccess(); } // Flag about erroneous recent operation
inline bool getBusStop() { return _busStatus.busStop; };  // Flag about current bus releasing


private:
//------------------------------------------------------------------------------
// Private constants
//------------------------------------------------------------------------------
enum AddressRange
{
  ADDRESS_MIN = 0x00,  // Minimal valid address
  ADDRESS_MAX = 0x7F,  // Maximal valid address
  ADDRESS_MIN_SPECIAL = 0x01,  // Minimal special purposes address
  ADDRESS_MIN_USUAL = 0x03,  // Minimal usual address
  ADDRESS_MAX_USUAL = 0x77,  // Maximal usual address
  ADDRESS_GENCALL = 0x00,  // General call address
};
enum GeneralCall
{
  GENCALL_RESET = 0x06,  // Software reset and write programmable part of slave address
  GENCALL_WRITE = 0x04,  // Write programmable part of slave address only
};


//------------------------------------------------------------------------------
// Private attributes
//------------------------------------------------------------------------------
struct
{
  uint8_t lastResult; // Result of a recent operation
  uint8_t lastCommand;  // Command code recently sent to two-wire bus
  uint8_t address = 255;  // Address of the device on two-wire bus
  uint32_t clock;  // Clock frequency in Hz
  bool busStop;  // Flag about releasing bus after end of transmission
  uint8_t pinSDA;  // Pin for serial data
  uint8_t pinSCL;  // Pin for serial clock
  uint32_t delaySend = 0;  // Waiting after each sent page to bus
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  bool busEnabled;  // Flag about bus initialization
#endif
} _busStatus;  // Microcontroller status features


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
inline uint8_t setLastCommand(uint8_t lastCommand) { return _busStatus.lastCommand = lastCommand; };
uint8_t platformWrite(uint8_t data);


protected:
//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------
inline void setDelaySend(uint32_t delay) { _busStatus.delaySend = delay; };  // Sending page delay in milliseconds
inline uint32_t getDelaySend() { return _busStatus.delaySend; };
/*
  Wait for a period of time.

  DESCRIPTION:
  The method wait in the loop until input delay expires.

  PARAMETERS:
  delay - Waiting time period in milliseconds.
          - Data type: non-negative integer
          - Default value: none
          - Limited range: 0 ~ 2^32 - 1

  RETURN: none
*/
void wait(uint32_t delay);


/*
  Initialize two-wire bus if it is not yet.

  DESCRIPTION:
  The method starts two-wire bus, if it is not yet and sets up the flag
  about it in order not to start the bus again.

  PARAMETERS: none

  RETURN: none
*/
void initBus();

};

#endif
