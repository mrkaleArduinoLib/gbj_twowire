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

  RETURN:  object
*/
gbj_twowire(uint32_t clockSpeed = CLOCK_100KHZ, bool busStop = true);


/*
  Destructor.

  DESCRIPTION:
  Destructor releases two-wire bus.

  PARAMETERS: none

  RETURN:  none
*/
~gbj_twowire();


/*
  Release two-wire bus.

  DESCRIPTION:
  The pins used by the two-wire bus are available for general purpose I/O.

  PARAMETERS: none

  RETURN: none
*/
void release();


/*
  Write one or two bytes to the two-wire bus.

  DESCRIPTION:
  The method writes one or two byte integer data to the two-wire bus in respect
  to the current platform.
  - If the most significant byte (the first one from the left) is non-zero,
    the data is written as two subsequent bytes.
  - If the most significant byte is zero, the data is written as its Least
    significant byte (the right most one).

  PARAMETERS:
  data - Data word or byte to be written.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: 0 ~ 65535

  RETURN:
  Number of transmitted bytes.
*/
uint8_t busWrite(uint16_t data);


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


//------------------------------------------------------------------------------
// Public setters - they usually return result code.
//------------------------------------------------------------------------------
inline void initLastResult() { _status.lastResult = SUCCESS; };
inline uint8_t setLastResult(uint8_t lastResult = SUCCESS) { return _status.lastResult = lastResult; };
inline void setBusStop(bool busStop) { _status.busStop = busStop; };
uint8_t setAddress(uint8_t address);

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
inline uint8_t getLastResult() { return _status.lastResult; }; // Result of a recent operation
inline uint8_t getLastCommand() { return _status.lastCommand; }; // Command code of a recent operation
inline uint8_t getAddress() { return _status.address; };  // Current device address
inline uint8_t getAddressMin() { return ADDRESS_MIN; };  // Adress limits...
inline uint8_t getAddressMax() { return ADDRESS_MAX; };
inline uint8_t getAddressMinSpecial() { return ADDRESS_MIN_SPECIAL; };
inline uint8_t getAddressMinUsual() { return ADDRESS_MIN_USUAL; };
inline uint8_t getAddressMaxUsual() { return ADDRESS_MAX_USUAL; };
inline uint32_t getBusClock() { return _status.clock; };  // Bus clock frequency in Hz
inline bool isSuccess() { return _status.lastResult == SUCCESS; } // Flag about successful recent operation
inline bool isError() { return !isSuccess(); } // Flag about erroneous recent operation
inline bool getBusStop() { return _status.busStop; };  // Flag about current bus releasing


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
};


//------------------------------------------------------------------------------
// Private attributes
//------------------------------------------------------------------------------
struct
{
  uint8_t lastResult; // Result of a recent operation
  uint8_t lastCommand;  // Command code recently sent to two-wire bus
  uint8_t address;  // Address of the device on two-wire bus
  uint32_t clock;  // Clock frequency in Hz
  bool busStop;  // Flag about releasing bus after end of transmission
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  bool busEnabled;  // Flag about bus initialization
#endif
} _status;  // Microcontroller status features


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
inline uint8_t setLastCommand(uint8_t lastCommand) { return _status.lastCommand = lastCommand; };
uint8_t platformWrite(uint8_t data);


protected:
//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------

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
