/*
  NAME:
  gbjTwoWire

  DESCRIPTION:
  Library embraces and provides common methods used at every application
  working with sensor on two-wire (I2C) bus.
  - Library specifies (inherits from) the system TwoWire library.
  - Library implements extended error handling.
  - Library provides some general system methods implemented differently for
    various platforms, especially Arduino, ESP8266, ESP32, and Particle.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the license GNU GPL v3
  http://www.gnu.org/licenses/gpl-3.0.html (related to original code) and MIT
  License (MIT) for added code.

  CREDENTIALS:
  Author: Libor Gabaj
  GitHub: https://github.com/mrkaleArduinoLib/gbj_twowire.git
*/
#ifndef GBJ_TWOWIRE_H
#define GBJ_TWOWIRE_H

#if defined(__AVR__)
  #include <Arduino.h>
  #include <Wire.h>
  #include <inttypes.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <Arduino.h>
  #include <Wire.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#endif

class gbj_twowire : public TwoWire
{
public:
  const bool REVERSE = true;
  enum ResultCodes : uint8_t
  {
    SUCCESS = 0,
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    // Data too long to fit in transmit buffer
    ERROR_BUFFER = 1,
    // Received NACK on transmit of address
    ERROR_NACK_ADDR = 2,
    // Received NACK on transmit of data
    ERROR_NACK_DATA = 3,
    // Other error
    ERROR_NACK_OTHER = 4,
    // Arduino custom errors
    ERROR_ADDRESS = ERROR_NACK_ADDR,
#elif defined(PARTICLE)
    // Busy timeout upon entering endTransmission()
    ERROR_BUSY = 1,
    // START bit generation timeout
    ERROR_START = 2,
    // End of address transmission timeout
    ERROR_END = 3,
    // Data byte transfer timeout
    ERROR_TRANSFER = 4,
    // Data byte transfer succeeded, busy timeout immediately after
    ERROR_TIMEOUT = 5,
    // Particle custom errors
    ERROR_ADDRESS = ERROR_START,
#endif
    // Error defining pins, usually both are the same
    ERROR_PINS = 255,
    // Less data received than expected
    ERROR_RCV_DATA = 254,
    // Wrong position in memory; either 0 or no sufficient space for data
    // storing or retrieving
    ERROR_POSITION = 253,
    // Wrong device type or other device fault
    ERROR_DEVICE = 252,
    // Device reset failure
    ERROR_RESET = 251,
    // Firmware reading failure
    ERROR_FIRMWARE = 250,
    // Serial number reading failure
    ERROR_SN = 249,
    // Measuring failure
    ERROR_MEASURE = 248,
    // Operation with a register failure
    ERROR_REGISTER = 247,
  };
  enum ClockSpeeds : uint32_t
  {
    CLOCK_100KHZ = 100000L,
    CLOCK_400KHZ = 400000L,
  };

  /*
    Constructor.

    DESCRIPTION:
    Constructor creates the class instance object and sets some bus parameters.
    Those parameters can be changed individually later in a sketch, if needed
    to change them dynamically.

    PARAMETERS:
    clockSpeed - Initial two-wire bus clock frequency in Hertz. If the clock is
    not from enumeration, it fallbacks to 100 kHz.
      - Data type: ClockSpeeds
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

    RETURN: object
  */
  inline gbj_twowire(ClockSpeeds clockSpeed = ClockSpeeds::CLOCK_100KHZ,
                     uint8_t pinSDA = 4,
                     uint8_t pinSCL = 5)
  {
    _busStatus.clock = clockSpeed;
    setPins(pinSDA, pinSCL);
    setBusStop();
    setStreamDirDft();
    setStreamBytesDft();
  }

  /*
    Destructor.

    DESCRIPTION:
    Destructor releases two-wire bus.

    PARAMETERS: none

    RETURN: none
  */
  inline ~gbj_twowire() { release(); }

  /*
    Initialize two wire bus and check parameters stored by constructor.

    DESCRIPTION:
    The method validates pin definitions from constructor for software defined
    I2C bus microcontrollers (ESP8266, ESP32).

    PARAMETERS: none

    RETURN: Result code
  */
  inline ResultCodes begin()
  {
    initBus();
    // Check pin duplicity
    if (_busStatus.pinSDA == _busStatus.pinSCL)
    {
      return setLastResult(ResultCodes::ERROR_PINS);
    }
    return getLastResult();
  }

  /*
    Release two-wire bus.

    DESCRIPTION:
    The pins used by the two-wire bus are available for general purpose I/O.

    PARAMETERS: none

    RETURN: none
  */
  inline void release()
  {
#if defined(__AVR__) || defined(PARTICLE)
    end();
#endif
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    _busStatus.busEnabled = false;
#endif
  }

  /*
    Send byte stream to the two-wire bus.

    DESCRIPTION:
    The method sends input data byte stream to the two-wire bus chunked by
    parent library two-wire data buffer length (paging).
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

    RETURN: Result code
  */
  ResultCodes busSendStream(uint8_t *dataBuffer,
                            uint16_t dataLen,
                            bool dataReverse = false);

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

    prfxReverse - Flag about sending the prefix buffer in reverse order from
    very last byte (determined by prfxLen) to the very first byte.
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

    RETURN: Result code
  */
  ResultCodes busSendStreamPrefixed(uint8_t *dataBuffer,
                                    uint16_t dataLen,
                                    bool dataReverse,
                                    uint8_t *prfxBuffer,
                                    uint16_t prfxLen,
                                    bool prfxReverse,
                                    bool prfxOnetime = false);

  /*
    Send one or two bytes to the two-wire bus.

    DESCRIPTION:
    The method sends input data to the two-wire bus as one communication
    transmission.
    - The method is overloaded.
    - In case of two parameters, the first one is considered as a command and
    second one as the data. In this case the method sends 2 ~ 4 bytes to the bus
    in one transmission.
    - In case of one parameter, it is considered as a command, but it can be the
    general data. In this case the method sends 1 ~ 2 bytes to the bus in one
    transmission.
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

    RETURN: Result code
  */
  inline ResultCodes busSend(uint16_t command)
  {
    uint8_t dataBuffer[2];
    uint16_t dataLen = 0;
    bufferData(dataBuffer, dataLen, setLastCommand(command));
    return busSendStream(dataBuffer, dataLen);
  }

  inline ResultCodes busSend(uint16_t command, uint16_t data)
  {
    uint8_t dataBuffer[4];
    uint16_t dataLen = 0;
    bufferData(dataBuffer, dataLen, setLastCommand(command));
    bufferData(dataBuffer, dataLen, data);
    return busSendStream(dataBuffer, dataLen);
  }

  /*
    Read byte stream from the two-wire bus.

    DESCRIPTION:
    The method reads a byte stream from the two-wire bus chunked by parent
    library two-wire data buffer length (paging) and places them to the buffer
    defined by an input pointer.
    - The method is overloaded.
    - If the first simple command parameter is present, the method combines
      sending this command to a device at first with subsequent reading from it.

    PARAMETERS:
    command - Word or byte to be sent in the role of command. If present, this
    command is sent to the two-wire bus with repeated start condition right
    before reading from the bus with stop condition.
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

    dataReverse - Flag about receiving to the data buffer in reverse order from
    very last byte (determined by dataLen) to the very first byte.
      - Data type: boolean
      - Default value: false
      - Limited range:
        false: receiving from the first to the last byte order
        true: receiving from the last to the first byte order

    RETURN: Result code
  */
  ResultCodes busReceive(uint8_t *dataBuffer,
                         uint16_t dataLen,
                         bool dataReverse = false);
  ResultCodes busReceive(uint16_t command,
                         uint8_t *dataBuffer,
                         uint16_t dataLen,
                         bool dataReverse = false);

  /*
    Send software reset command to general call address.

    DESCRIPTION:
    The method sends command 0x06 to the general call address 0x00 in order to
    execute software reset of all devices on the two-wire bus that have this
    functionality implemented.

    PARAMETERS: none

    RETURN: Result code
  */
  inline ResultCodes busGeneralReset()
  {
    initBus();
    beginTransmission(AddressRange::ADDRESS_GENCALL);
    write(GeneralCall::GENCALL_RESET);
    if (setLastResult(static_cast<ResultCodes>(endTransmission(getBusStop()))))
    {
      return getLastResult();
    }
    setTimestamp();
    return getLastResult();
  }

  // Setters
  inline ResultCodes registerAddress(uint8_t address)
  {
    setLastResult();
    // Invalid address
    if (address < AddressRange::ADDRESS_MIN ||
        address > AddressRange::ADDRESS_MAX)
    {
      return setLastResult(ResultCodes::ERROR_ADDRESS);
    }
    // No address change
    if (address == getAddress())
    {
      return getLastResult();
    }
    // Set changed address
    _busStatus.address = address;
    return getLastResult();
  }

  inline ResultCodes setAddress(uint8_t address)
  {
    if (isError(registerAddress(address)))
    {
      return getLastResult();
    }
    beginTransmission(getAddress());
    return setLastResult(
      static_cast<ResultCodes>(endTransmission(getBusStop())));
  }

  inline ResultCodes setPins(uint8_t pinSDA, uint8_t pinSCL)
  {
    _busStatus.pinSDA = pinSDA;
    _busStatus.pinSCL = pinSCL;
    setLastResult();
    // Check pin duplicity
    if (_busStatus.pinSDA == _busStatus.pinSCL)
    {
      return setLastResult(ResultCodes::ERROR_PINS);
    }
    return getLastResult();
  }

  inline ResultCodes setLastResult(
    ResultCodes lastResult = ResultCodes::SUCCESS)
  {
    if (lastResult != ResultCodes::SUCCESS)
    {
      setBusStop();
    }
    _busStatus.lastResult = lastResult;
    return _busStatus.lastResult;
  }

  /*
    Set frequency of the two-wire bus.

    DESCRIPTION:
    Method updates the bus clock frequency in the class instance object only. It
    takes effect at next bus initialization by using method busSend or
    busReceive.

    PARAMETERS:
    clockSpeed - Two-wire bus clock frequency in Hertz. If the clock is not from
    enumeration, it fallbacks to 100 kHz.
      - Data type: long
      - Default value: CLOCK_100KHZ
      - Limited range: CLOCK_100KHZ, CLOCK_400KHZ

    RETURN: none
  */
  inline void setBusClock(ClockSpeeds clockSpeed)
  {
    switch (clockSpeed)
    {
      case ClockSpeeds::CLOCK_100KHZ:
      case ClockSpeeds::CLOCK_400KHZ:
        _busStatus.clock = clockSpeed;
        break;
      default:
        _busStatus.clock = ClockSpeeds::CLOCK_100KHZ;
        break;
    };
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    setClock(_busStatus.clock);
#elif defined(PARTICLE)
    setSpeed(_busStatus.clock);
#endif
  }

  // Getters
  inline ResultCodes getLastResult()
  {
    if (_busStatus.lastResult != ResultCodes::SUCCESS)
    {
      setBusStop();
    }
    return _busStatus.lastResult;
  }
  inline uint8_t getAddress() { return _busStatus.address; };
  // Adress limits
  inline uint8_t getAddressMin() { return AddressRange::ADDRESS_MIN; };
  inline uint8_t getAddressMax() { return AddressRange::ADDRESS_MAX; };
  inline uint8_t getAddressMinSpecial()
  {
    return AddressRange::ADDRESS_MIN_SPECIAL;
  };
  inline uint8_t getAddressMinUsual()
  {
    return AddressRange::ADDRESS_MIN_USUAL;
  };
  inline uint8_t getAddressMaxUsual()
  {
    return AddressRange::ADDRESS_MAX_USUAL;
  };
  inline uint8_t getPinSDA() { return _busStatus.pinSDA; };
  inline uint8_t getPinSCL() { return _busStatus.pinSCL; };
  inline uint16_t getLastCommand() { return _busStatus.lastCommand; };
  // Bus clock frequency in Hz
  inline ClockSpeeds getBusClock() { return _busStatus.clock; }

  /*
    Flag about successful or erroneous recent operation.

    DESCRIPTION:
    Method is overloaded.

    PARAMETERS:
    lastResult - Directly input result code usually returned from inline method
    or function. At absence of the input argument the stored code of recent
    operation is used.
      - Data type: ResultCodes
      - Default value: none
      - Limited range: enumeration

    RETURN: Boolean flag about success of the recent operation
  */
  inline bool isSuccess()
  {
    return _busStatus.lastResult == ResultCodes::SUCCESS;
  }
  inline bool isSuccess(ResultCodes lastResult)
  {
    _busStatus.lastResult = lastResult;
    return isSuccess();
  }
  inline bool isError() { return !isSuccess(); }
  inline bool isError(ResultCodes lastResult) { return !isSuccess(lastResult); }

  /*
    Convert error code to text.

    DESCRIPTION:
    Method translates internally stored error code of the recent operation to
    corresponding wording.

    PARAMETERS:
    location - Location of the error code in a sketch utilized as an error text
    prefix.
      - Data type: String
      - Default value: empty string
      - Limited range: none

    RETURN: Textual wording of an error code
  */
  String getLastErrorTxt(String location = "");

  /*
    Set delay for waiting before each sending transmission.

    DESCRIPTION:
    If a delay value is set, than the library waits before subsequent sending
    transmission until that time period expires from finishing previous
    transmission.

    PARAMETERS:
    delay - Delaying time period in milliseconds.
      - Data type: non-negative integer
      - Default value: none
      - Limited range: 0 ~ 2^32 - 1

    RETURN: none
  */
  inline void setDelaySend(uint32_t delay) { _busStatus.sendDelay = delay; }
  inline uint32_t getDelaySend() { return _busStatus.sendDelay; }

  /*
    Set delay for waiting before each receiving transmission.

    DESCRIPTION:
    If a delay value is set, than the library waits before subsequent receiving
    transmission until that time period expires from finishing previous
    transmission.

    PARAMETERS:
    delay - Delaying time period in milliseconds.
      - Data type: non-negative integer
      - Default value: none
      - Limited range: 0 ~ 2^32 - 1

    RETURN: none
  */
  inline void setDelayReceive(uint32_t delay)
  {
    _busStatus.receiveDelay = delay;
  }
  inline uint32_t getDelayReceive() { return _busStatus.receiveDelay; }

private:
  enum AddressRange : uint8_t
  {
    ADDRESS_MIN = 0x00, // Minimal valid address
    ADDRESS_MAX = 0x7F, // Maximal valid address
    ADDRESS_MIN_SPECIAL = 0x01, // Minimal special purposes address
    ADDRESS_MIN_USUAL = 0x03, // Minimal usual address
    ADDRESS_MAX_USUAL = 0x77, // Maximal usual address
    ADDRESS_GENCALL = 0x00, // General call address
  };
  enum GeneralCall : uint8_t
  {
    // Software reset and write programmable part of slave address
    GENCALL_RESET = 0x06,
    // Write programmable part of slave address only
    GENCALL_WRITE = 0x04,
  };
  enum DataStreamProcessing : uint8_t
  {
    // Process data stream with the least significant byte first
    STREAM_DIR_LSB = 0,
    // Process data stream with the most significant byte first
    STREAM_DIR_MSB = 1,
    // Process only non-zero bytes from LSB - ignore zero MSB bytes
    STREAM_BYTES_VAL = 2,
    // Process all bytes of data stream regardless of their value
    STREAM_BYTES_ALL = 3,
    // I2C Buffer length adopted from parent library
    STREAM_BUFFER_LENGTH = BUFFER_LENGTH,
  };
  struct BusStatus
  {
    ResultCodes lastResult; // Result of a recent operation
    uint16_t lastCommand; // Command code recently sent to two-wire bus
    uint8_t address = 255; // Address of the device on two-wire bus
    ClockSpeeds clock; // Clock frequency in Hz
    bool busStop; // Flag about releasing bus after end of transmission
    uint8_t pinSDA; // Pin for serial data
    uint8_t pinSCL; // Pin for serial clock
    uint8_t streamDirection; // Mode of data stream processing
    uint8_t streamBytes; // Mode of data stream bytes processing
    uint32_t sendDelay = 0; // Waiting after each sent page
    uint32_t receiveDelay = 0; // Waiting after each received page
    uint32_t transTimestamp = 0; // Recent bus transmission timestamp
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    bool busEnabled; // Flag about bus initialization
#endif
  } _busStatus; // Microcontroller status features

  inline uint16_t setLastCommand(uint16_t lastCommand)
  {
    return _busStatus.lastCommand = lastCommand;
  }

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
  inline void bufferData(uint8_t *dataBuffer, uint16_t &dataIdx, uint16_t data)
  {
    uint8_t dataLSB = data & 0xFF;
    uint8_t dataMSB = (data >> 8) & 0xFF;
    switch (getStreamDir())
    {
      case DataStreamProcessing::STREAM_DIR_MSB:
        if ((getStreamBytes() == DataStreamProcessing::STREAM_BYTES_ALL) ||
            dataMSB)
        {
          dataBuffer[dataIdx++] = dataMSB;
        }
        dataBuffer[dataIdx++] = dataLSB;
        break;
      case DataStreamProcessing::STREAM_DIR_LSB:
      default:
        if ((getStreamBytes() == DataStreamProcessing::STREAM_BYTES_ALL) ||
            dataLSB)
        {
          dataBuffer[dataIdx++] = dataLSB;
        }
        dataBuffer[dataIdx++] = dataMSB;
        break;
    }
  }

protected:
  inline void setBusStopFlag(bool busStop) { _busStatus.busStop = busStop; }
  inline void setBusStop() { setBusStopFlag(true); }
  inline void setBusRepeat() { setBusStopFlag(false); }
  inline bool getBusStop() { return _busStatus.busStop; }
  inline uint8_t getStreamDir() { return _busStatus.streamDirection; }
  inline void setStreamDirLSB()
  {
    _busStatus.streamDirection = DataStreamProcessing::STREAM_DIR_LSB;
  }
  inline void setStreamDirMSB()
  {
    _busStatus.streamDirection = DataStreamProcessing::STREAM_DIR_MSB;
  }
  inline void setStreamDirDft() { setStreamDirMSB(); }
  inline uint8_t getStreamBytes() { return _busStatus.streamBytes; }
  inline void setStreamBytesVal()
  {
    _busStatus.streamBytes = DataStreamProcessing::STREAM_BYTES_VAL;
  }
  inline void setStreamBytesAll()
  {
    _busStatus.streamBytes = DataStreamProcessing::STREAM_BYTES_ALL;
  }
  inline void setStreamBytesDft() { setStreamBytesVal(); }
  inline void setTimestamp(uint32_t timestamp = millis())
  {
    _busStatus.transTimestamp = timestamp;
  }
  inline uint32_t getTimestamp() { return _busStatus.transTimestamp; }
  inline void waitTimestampSend()
  {
    while (millis() - _busStatus.transTimestamp < getDelaySend())
      ;
  }
  inline void waitTimestampReceive()
  {
    while (millis() - _busStatus.transTimestamp < getDelayReceive())
      ;
  }
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
  inline void wait(uint32_t delay)
  {
    uint32_t timestamp = millis();
    while (millis() - timestamp < delay)
    {
      ;
    }
  }
  /*
    Initialize two-wire bus if it is not yet.

    DESCRIPTION:
    The method starts two-wire bus, if it is not yet and sets up the flag
    about it in order not to start the bus again.

    PARAMETERS: none

    RETURN: none
  */
  inline void initBus()
  {
    setLastResult();
#if defined(__AVR__)
    if (!_busStatus.busEnabled)
    {
      Wire.begin();
      _busStatus.busEnabled = true;
    }
#elif defined(ESP8266) || defined(ESP32)
    if (!_busStatus.busEnabled)
    {
      Wire.begin(_status.pinSDA, _status.pinSCL);
      _status.busEnabled = true;
    }
#elif defined(PARTICLE)
    if (!isEnabled())
    {
      Wire.begin();
    }
#endif
    setBusClock(getBusClock());
  }
};

#endif
