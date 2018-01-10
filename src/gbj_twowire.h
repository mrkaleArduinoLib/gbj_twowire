/*
  NAME:
  gbj_twowire

  DESCRIPTION:
  Library embraces and provides common methods used at every application
  working with sensor ontwo wire (I2C) bus.
  - Library specifies (inherits from) the system TwoWire library.
  - Library implements extended error handling.
  - Library provides some general system methods implemented differently for
    various platforms, especially Arduino vs. Particle.

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
#define GBJ_TWOWIRE_VERSION "GBJ_TWOWIRE 1.0.0"

#if defined(__AVR__)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
  #include <inttypes.h>
  #include <Wire.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#endif


// Addresses
#define GBJ_TWOWIRE_ADDRESS_MIN         0x01  // Minimal valid address
#define GBJ_TWOWIRE_ADDRESS_MAX         0x77  // Maximal valid address

#define GBJ_TWOWIRE_SUCCESS             0     // Result code

// Error codes
#ifndef GBJ_TWOWIRE_ERRORS_H
#define GBJ_TWOWIRE_ERRORS_H
  #if defined(__AVR__)
    // Arduino error codes
    #define GBJ_TWOWIRE_ERR_BUFFER      1   // Data too long to fit in transmit buffer
    #define GBJ_TWOWIRE_ERR_NACK_ADDR   2   // Received NACK on transmit of address
    #define GBJ_TWOWIRE_ERR_NACK_DATA   3   // Received NACK on transmit of data
    #define GBJ_TWOWIRE_ERR_OTHER       4   // Other error
    // Custom errors
    #define GBJ_TWOWIRE_ERR_ADDRESS     GBJ_TWOWIRE_ERR_NACK_ADDR
    // Arduino clock speed
    #define CLOCK_SPEED_100KHZ          100000L
    #define CLOCK_SPEED_400KHZ          400000L
  #elif defined(PARTICLE)
    // Particle error codes
    #define GBJ_TWOWIRE_ERR_BUSY        1   // Busy timeout upon entering endTransmission()
    #define GBJ_TWOWIRE_ERR_START       2   // START bit generation timeout
    #define GBJ_TWOWIRE_ERR_END         3   // End of address transmission timeout
    #define GBJ_TWOWIRE_ERR_TRANSFER    4   // Data byte transfer timeout
    #define GBJ_TWOWIRE_ERR_TIMEOUT     5   // Data byte transfer succeeded, busy timeout immediately after
    // Custom errors
    #define GBJ_TWOWIRE_ERR_ADDRESS     GBJ_TWOWIRE_ERR_START
  #endif
#endif


class gbj_twowire : protected TwoWire
{
public:
//------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------

/*
  Constructor.

  DESCRIPTION:
  Constructor just creates the class instance object.

  PARAMETERS: none

  RETURN:  object
*/
gbj_twowire();


/*
  Destructor.

  DESCRIPTION:
  Destructor releases two wire bus.

  PARAMETERS: none

  RETURN:  none
*/
~gbj_twowire();


/*
  Release two wire bus.

  DESCRIPTION:
  The pins used by the two wire bus are available for general purpose I/O.

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
uint8_t setAddress(uint8_t address);
bool    setBusStop(bool busStop);
uint8_t setLastResult(uint8_t lastResult = GBJ_TWOWIRE_SUCCESS);
void    initLastResult();


//------------------------------------------------------------------------------
// Public getters
//------------------------------------------------------------------------------
bool     getBusStop();        // Flag about current bus releasing
uint8_t  getAddress();        // Current device address
uint8_t  getLastResult();     // Result of a recent operation
bool     isSuccess();         // Flag about succsssful recent operation
bool     isError();           // Flag about erroneous recent operation


private:
//------------------------------------------------------------------------------
// Private attributes
//------------------------------------------------------------------------------
#if defined(__AVR__)
bool     _busEnabled;   // Flag about bus initialization
#endif
bool     _busStop;      // Flag about releasing bus after end of transmission
uint8_t  _address;      // Address of the sensor
uint8_t  _lastResult;   // Result of a recent operation


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------


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
  Initialize two wire bus if it is not yet.

  DESCRIPTION:
  The method starts two wire bus, if it is not yet and sets up the flag
  about it in order not to start the bus again.

  PARAMETERS: none

  RETURN: none
*/
void initBus();


private:
//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
uint8_t platformWrite(uint8_t data);

};

#endif
