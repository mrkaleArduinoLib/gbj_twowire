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
  ERROR_RCV_DATA = 11, // Less data received than expected
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
  clockSpeed - Initial two-wire bus clock frequency in Hertz. If the clock is
               not from enumeration, it fallbacks to 100 kHz.
               - Data type: long
               - Default value: CLOCK_100KHZ
               - Limited range: CLOCK_100KHZ, CLOCK_400KHZ

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
gbj_twowire(uint32_t clockSpeed = CLOCK_100KHZ, uint8_t pinSDA = 4, uint8_t pinSCL = 5);


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
  The method sends input data byte stream to the two-wire bus chunked by parent
  library two-wire data buffer length (paging).
  - If there is a send delay defined, the method waits for that time period
    expiration before sending next chunk (page).

  PARAMETERS:
  dataBuffer - Pointer to the byte data buffer.
               - Data type: non-negative integer
               - Default value: none
               - Limited range: address space

  dataLen - Number of bytes to be sent from the data buffer to the bus.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  dataReverse - Flag about sending the data buffer in reverse order from very
                last byte (determined by dataLen) to the very first byte.
                - Data type: boolean
                - Default value: false
                - Limited range:
                  false: sending from the first to the last byte order
                  true: sending from the last to the first byte order

  RETURN:
  Result code.
*/
uint8_t busSendStream(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse = false);


/*
  Send prefixed byte stream to the two-wire bus.

  DESCRIPTION:
  The method sends input data byte array to the two-wire prefixed with prefix
  buffer.
  - The method chunks sent byte stream including prefix by parent library
    two-wire data buffer length (paging).
  - If there is a send delay defined, the method waits for that time period
    expiration before sending next chunk (page).
  - The prefix may be a one-time one, which is used just with the very first
    chunk only.

  PARAMETERS:
  dataBuffer - Pointer to the byte data buffer.
               - Data type: non-negative integer
               - Default value: none
               - Limited range: address space

  dataLen - Number of bytes to be sent from the data buffer to the bus.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  dataReverse - Flag about sending the data buffer in reverse order from very
                last byte (determined by dataLen) to the very first byte.
                - Data type: boolean
                - Default value: none
                - Limited range:
                  false: sending from the first to the last byte order
                  true: sending from the last to the first byte order

  prfxBuffer - Pointer to the prefix byte buffer, which precedes sending each
               or the first data buffer page.
               - Data type: non-negative integer
               - Default value: none
               - Limited range: address space

  prfxLen - Number of bytes to be sent from the prefix buffer to the bus.
            At repeating prefix (non one-time) it is reasonable, if the prefix
            length is less than two-wire buffer length (usually 32 bytes),
            otherwise only that prefix is sent to the bus due to paging.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  prfxReverse - Flag about sending the prefix buffer in reverse order from very
                last byte (determined by prfxLen) to the very first byte.
                - Data type: boolean
                - Default value: none
                - Limited range:
                  false: sending from the first to the last byte order
                  true: sending from the last to the first byte order

  prfxOnetime - Flag about sending the prefix buffer just once before the very
                first sending the data buffer.
                - Data type: boolean
                - Default value: false
                - Limited range:
                  false: prefix buffer sent before each data buffer page
                  true: prefix buffer sent once before start of sending data
                        buffer

  RETURN:
  Result code.
*/
uint8_t busSendStreamPrefixed(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse, \
  uint8_t *prfxBuffer, uint16_t prfxLen, bool prfxReverse, bool prfxOnetime = false);


/*
  Send one or two bytes to the two-wire bus.

  DESCRIPTION:
  The method sends input data to the two-wire bus as one communication
  transmission.
  - The method is overloaded.
  - In case of two parameters, the first one is considered as a command and
    second one as the data. In this case the method sends 2 ~ 4 bytes to the bus
    in one transmission.
  - In case of one parameter, it is considered as the general data and in fact
    might be a command or the data. In this case the method sends 1 ~ 2 bytes
    to the bus in one transmission.
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
uint8_t busSend(uint16_t data);
uint8_t busSend(uint16_t command, uint16_t data);


/*
  Read byte stream from the two-wire bus.

  DESCRIPTION:
  The method reads a byte stream from the two-wire bus chunked by parent
  library two-wire data buffer length (paging) and places them to the
  buffer defined by an input pointer.
  - The method is overloaded.
  - If the first simple command parameter is present, the method combines
    sending this command to a device at first with subsequent reading from it.

  PARAMETERS:
  command - Word or byte to be sent in the role of command. If present, this
            command is sent to the two-wire bus with repeated start condition
            right before reading from the bus with stop condition.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  dataBuffer - Pointer to a byte buffer for storing read data. The buffer
               should be enough large for storing all read bytes.
               - Data type: non-negative integer
               - Default value: none
               - Limited range: address space

  dataLen - Number of bytes to be read.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  RETURN:
  Result code.
*/
uint8_t busReceive(uint8_t *dataBuffer, uint16_t dataLen);
uint8_t busReceive(uint16_t command, uint8_t *dataBuffer, uint16_t dataLen);


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
inline uint8_t initLastResult() { return _busStatus.lastResult = SUCCESS; };
uint8_t registerAddress(uint8_t address);
uint8_t setAddress(uint8_t address);
uint8_t setPins(uint8_t pinSDA, uint8_t pinSCL);
uint8_t setLastResult(uint8_t lastResult = SUCCESS);

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
uint8_t getLastResult();
inline uint8_t getAddress() { return _busStatus.address; };
inline uint8_t getAddressMin() { return ADDRESS_MIN; };  // Adress limits...
inline uint8_t getAddressMax() { return ADDRESS_MAX; };
inline uint8_t getAddressMinSpecial() { return ADDRESS_MIN_SPECIAL; };
inline uint8_t getAddressMinUsual() { return ADDRESS_MIN_USUAL; };
inline uint8_t getAddressMaxUsual() { return ADDRESS_MAX_USUAL; };
inline uint8_t getPinSDA() { return _busStatus.pinSDA; };
inline uint8_t getPinSCL() { return _busStatus.pinSCL; };
inline uint16_t getLastCommand() { return _busStatus.lastCommand; };
inline uint32_t getBusClock() { return _busStatus.clock; };  // Bus clock frequency in Hz
inline bool isSuccess() { return _busStatus.lastResult == SUCCESS; };  // Flag about successful recent operation
inline bool isError() { return !isSuccess(); };  // Flag about erroneous recent operation


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
enum DataStreamProcessing
{
  STREAM_DIR_LSB = 0,  // Process data stream with the least significant byte first
  STREAM_DIR_MSB = 1,  // Process data stream with the most significant byte first
  STREAM_BYTES_VAL = 2,  // Process only non-zero bytes from LSB - ignore zero MSB bytes
  STREAM_BYTES_ALL = 3,  // Process all bytes of data stream regardless of their value
};


//------------------------------------------------------------------------------
// Private attributes
//------------------------------------------------------------------------------
struct
{
  uint8_t lastResult; // Result of a recent operation
  uint16_t lastCommand;  // Command code recently sent to two-wire bus
  uint8_t address = 255;  // Address of the device on two-wire bus
  uint32_t clock;  // Clock frequency in Hz
  bool busStop;  // Flag about releasing bus after end of transmission
  uint8_t pinSDA;  // Pin for serial data
  uint8_t pinSCL;  // Pin for serial clock
  uint8_t streamDirection;  // Mode of data stream processing
  uint8_t streamBytes;  // Mode of data stream bytes processing
  uint32_t sendDelay = 0;  // Waiting after each sent page to bus in milliseconds
  uint32_t sendTimestamp = 0;  // Timestamp of recent bus send in milliseconds
  uint32_t receiveDelay = 0;  // Waiting after each received page to bus in milliseconds
  uint32_t receiveTimestamp = 0;  // Timestamp of recent bus receive in milliseconds
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  bool busEnabled;  // Flag about bus initialization
#endif
} _busStatus;  // Microcontroller status features


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
inline uint16_t setLastCommand(uint16_t lastCommand) { return _busStatus.lastCommand = lastCommand; };
uint8_t platformWrite(uint8_t data);
uint8_t platformRead();


protected:
//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------
inline void setBusStopFlag(bool busStop) { _busStatus.busStop = busStop; };
inline void setBusStop() { setBusStopFlag(true); };
inline void setBusRpte() { setBusStopFlag(false); };  // Start repeated
inline bool getBusStop() { return _busStatus.busStop; };
inline uint8_t getStreamDir() { return _busStatus.streamDirection; };
inline void setStreamDirLSB() { _busStatus.streamDirection = STREAM_DIR_LSB; };
inline void setStreamDirMSB() { _busStatus.streamDirection = STREAM_DIR_MSB; };
inline void setStreamDirDflt() { setStreamDirMSB(); };
inline uint8_t getStreamBytes() { return _busStatus.streamBytes; };
inline void setStreamBytesVal() { _busStatus.streamBytes = STREAM_BYTES_VAL; };
inline void setStreamBytesAll() { _busStatus.streamBytes = STREAM_BYTES_ALL; };
inline void setStreamBytesDflt() { setStreamBytesVal(); };
/*
  Set delay for waiting after each sending transmission to settle a device.

  DESCRIPTION:
  If a delay value is set, than the library waits before subsequent sending
  transmission until that time period expires from finishing previous sending
  transmission.

  PARAMETERS:
  delay - Delaying time period in milliseconds.
          - Data type: non-negative integer
          - Default value: none
          - Limited range: 0 ~ 2^32 - 1

  RETURN: none
*/
inline void setDelaySend(uint32_t delay) { _busStatus.sendDelay = delay; };
inline void resetDelaySend() { _busStatus.sendDelay = 0; };
inline uint32_t getDelaySend() { return _busStatus.sendDelay; };
inline void setTimestampSend() { _busStatus.sendTimestamp = millis(); };
inline void resetTimestampSend() { _busStatus.sendTimestamp = 0; };
inline void waitTimestampSend() { while (millis() - _busStatus.sendTimestamp < getDelaySend()); };
inline uint32_t getTimestampSend() { return _busStatus.sendTimestamp; };
/*
  Set delay for waiting after each receiving transmission to settle a device.

  DESCRIPTION:
  If a delay value is set, than the library waits before subsequent receiving
  transmission until that time period expires from finishing previous receiving
  transmission.

  PARAMETERS:
  delay - Delaying time period in milliseconds.
          - Data type: non-negative integer
          - Default value: none
          - Limited range: 0 ~ 2^32 - 1

  RETURN: none
*/
inline void setDelayReceive(uint32_t delay) { _busStatus.receiveDelay = delay; };
inline void resetDelayReceive() { _busStatus.receiveDelay = 0; };
inline uint32_t getDelayReceive() { return _busStatus.receiveDelay; };
inline void setTimestampReceive() { _busStatus.receiveTimestamp = millis(); };
inline void resetTimestampReceive() { _busStatus.receiveTimestamp = 0; };
inline void waitTimestampReceive() { while (millis() - _busStatus.receiveTimestamp < getDelayReceive()); };
inline uint32_t getTimestampReceive() { return _busStatus.receiveTimestamp; };


/*
  Wait for a period of time.

  DESCRIPTION:
  The method waits in the loop until input delay expires.

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


/*
  Buffer data word.

  DESCRIPTION:
  The method updates provided buffer from provided index with provided data
  according to current streaming modes (STREAM_DIR_LSB ... STREAM_BYTES_VAL)
  and updates that index.

  PARAMETERS:
  dataBuffer - Pointer to the byte data buffer, which should be updated.
               - Data type: non-negative integer
               - Default value: none
               - Limited range: address space

  dataIdx - Index to the buffer where update should start.
            - Data type: non-negative integer
            - Default value: none
            - Limited range: 0 ~ 65535

  data - Data word, which should be analyzed and written to the buffer.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: 0 ~ 65535

  RETURN: None
*/
void bufferData(uint8_t *dataBuffer, uint16_t &dataIdx, uint16_t data);

};

#endif
