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
#define GBJ_TWOWIRE_ADDRESS_BAD         0xFF  // Value for bad address
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


class gbj_twowire : TwoWire
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

};

#endif
