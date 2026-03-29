/**
 * @file gbj_twowire.h
 * @brief Two-wire (I2C) bus driver base library.
 * @details Provides common methods for I2C communication across multiple
 * platforms (Arduino, ESP8266, ESP32, Particle). Features extended error
 * handling and platform-specific implementations for sensor libraries.
 *
 * @copyright This program is free software; you can redistribute it and/or
 * modify it under the terms of the license GNU GPL v3
 * http://www.gnu.org/licenses/gpl-3.0.html (related to original code) and MIT
 * License (MIT) for added code.
 *
 * @author Libor Gabaj
 * @see https://github.com/mrkaleArduinoLib/gbj_twowire.git
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

/**
 * @class gbj_twowire
 * @brief Two-wire (I2C) bus driver.
 * @details Extends Arduino TwoWire class with extended error handling,
 * multi-platform support, and data streaming capabilities.
 */
class gbj_twowire : public TwoWire
{
public:
  // Boolean tag determining reverse order of bytes in stream transmission
  // (default: true)
  const bool REVERSE = true;

  enum ResultCodes : uint8_t
  {
    SUCCESS = 0,
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    /// Data too long to fit in transmit buffer
    ERROR_BUFFER = 1,
    /// Received NACK on transmit of address
    ERROR_NACK_ADDR = 2,
    /// Received NACK on transmit of data
    ERROR_NACK_DATA = 3,
    /// Other error
    ERROR_NACK_OTHER = 4,
    /// Arduino custom errors
    ERROR_ADDRESS = ERROR_NACK_ADDR,
#elif defined(PARTICLE)
    /// Busy timeout upon entering endTransmission()
    ERROR_BUSY = 1,
    /// START bit generation timeout
    ERROR_START = 2,
    /// End of address transmission timeout
    ERROR_END = 3,
    /// Data byte transfer timeout
    ERROR_TRANSFER = 4,
    /// Data byte transfer succeeded, busy timeout immediately after
    ERROR_TIMEOUT = 5,
    /// Particle custom errors
    ERROR_ADDRESS = ERROR_START,
#endif
    /// Error defining pins, usually both are the same
    ERROR_PINS = 255,
    /// Less data received than expected
    ERROR_RCV_DATA = 254,
    /// Wrong position in memory; either 0 or no sufficient space for data
    ERROR_POSITION = 253,
    /// Wrong device type or other device fault
    ERROR_DEVICE = 252,
    /// Device reset failure
    ERROR_RESET = 251,
    /// Firmware reading failure
    ERROR_FIRMWARE = 250,
    /// Serial number reading failure
    ERROR_SN = 249,
    /// Measuring failure
    ERROR_MEASURE = 248,
    /// Operation with a register failure
    ERROR_REGISTER = 247,
  };

  enum ClockSpeeds : uint32_t
  {
    CLOCK_100KHZ = 100000L,
    CLOCK_400KHZ = 400000L,
  };

  /**
   * @brief Construct the two-wire driver object.
   * @details Sets initial bus parameters that can be modified later.
   * @param clockSpeed Initial I2C bus clock frequency (default: 100 kHz).
   * @param pinSDA Microcontroller pin for SDA (default: 4).
   * @param pinSCL Microcontroller pin for SCL (default: 5).
   */
  inline gbj_twowire(ClockSpeeds clockSpeed = ClockSpeeds::CLOCK_100KHZ,
                     uint8_t pinSDA = 4,
                     uint8_t pinSCL = 5)
  {
    busStatus_.clock = clockSpeed;
    setPins(pinSDA, pinSCL);
    setBusStop();
    setStreamDirDft();
    setStreamBytesDft();
  }

  /**
   * @brief Destroy the two-wire driver object.
   * @details Releases the I2C bus resources.
   */
  inline ~gbj_twowire() { release(); }

  /**
   * @brief Initialize the two-wire bus.
   * @details Validates pin definitions and prepares the bus for communication.
   * @return Result code (SUCCESS or ERROR_PINS).
   */
  inline ResultCodes begin()
  {
    initBus();
    // Check pin duplicity
    if (busStatus_.pinSDA == busStatus_.pinSCL)
    {
      return setLastResult(ResultCodes::ERROR_PINS);
    }
    return getLastResult();
  }

  /**
   * @brief Release the two-wire bus.
   * @details Frees I2C bus pins for general I/O use.
   */
  inline void release()
  {
#if defined(__AVR__) || defined(PARTICLE)
    end();
#endif
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    busStatus_.busEnabled = false;
#endif
  }

  /**
   * @brief Send byte stream to the I2C bus.
   * @details Transmits data in pages respecting the bus buffer size.
   * Supports send delay and reverse byte order transmission.
   * @param dataBuffer Pointer to data buffer to send.
   * @param dataLen Number of bytes to send.
   * @param dataReverse Send bytes in reverse order (default: false).
   * @return Result code.
   */
  ResultCodes busSendStream(uint8_t *dataBuffer,
                            uint16_t dataLen,
                            bool dataReverse = false);

  /**
   * @brief Send prefixed byte stream to the I2C bus.
   * @details Sends data prefixed with prefix buffer, chunked by bus buffer
   * size. Prefix can be sent once or repeated with each data page.
   * @param dataBuffer Pointer to data buffer.
   * @param dataLen Number of data bytes.
   * @param dataReverse Send data bytes in reverse order.
   * @param prfxBuffer Pointer to prefix buffer.
   * @param prfxLen Number of prefix bytes.
   * @param prfxReverse Send prefix bytes in reverse order.
   * @param prfxOnetime Send prefix only before first data page (default:
   * false).
   * @return Result code.
   */
  ResultCodes busSendStreamPrefixed(uint8_t *dataBuffer,
                                    uint16_t dataLen,
                                    bool dataReverse,
                                    uint8_t *prfxBuffer,
                                    uint16_t prfxLen,
                                    bool prfxReverse,
                                    bool prfxOnetime = false);

  /**
   * @brief Send one or two bytes to the I2C bus.
   * @details Overloaded method for simple command or command+data transmission.
   * MSB is automatically handled (sent only if non-zero).
   * @param command Byte or word to send as command.
   * @param data Optional data byte or word to send after command.
   * @return Result code.
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

  /**
   * @brief Read byte stream from the I2C bus.
   * @details Receives data in pages respecting bus buffer size.
   * Supports receive delay and optional command + read pattern.
   * @param command Optional command to send before reading (with repeated
   * start).
   * @param dataBuffer Pointer to buffer for storing received data.
   * @param dataLen Number of bytes to receive.
   * @param dataReverse Receive bytes in reverse order (default: false).
   * @return Result code.
   */
  ResultCodes busReceive(uint8_t *dataBuffer,
                         uint16_t dataLen,
                         bool dataReverse = false);
  ResultCodes busReceive(uint16_t command,
                         uint8_t *dataBuffer,
                         uint16_t dataLen,
                         bool dataReverse = false);

  /**
   * @brief Send general call software reset to all devices.
   * @details Sends reset command (0x06) to general call address (0x00)
   * to reset all devices on the bus that support it.
   * @return Result code.
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

  /// @name Setters
  /// @{
  /**
   * @brief Register device address without testing.
   * @details Validates address is within valid range.
   * @param address I2C address (0x00 - 0x7F).
   * @return Result code.
   */
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
    busStatus_.address = address;
    return getLastResult();
  }

  /**
   * @brief Set and test device address.
   * @details Sets address and verifies communication by attempting
   * transmission.
   * @param address I2C address (0x00 - 0x7F).
   * @return Result code.
   */
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

  /**
   * @brief Set I2C bus pins.
   * @details Configures SDA and SCL pins. Validates pins are different.
   * @param pinSDA Serial data pin number.
   * @param pinSCL Serial clock pin number.
   * @return Result code.
   */
  inline ResultCodes setPins(uint8_t pinSDA, uint8_t pinSCL)
  {
    busStatus_.pinSDA = pinSDA;
    busStatus_.pinSCL = pinSCL;
    setLastResult();
    // Check pin duplicity
    if (busStatus_.pinSDA == busStatus_.pinSCL)
    {
      return setLastResult(ResultCodes::ERROR_PINS);
    }
    return getLastResult();
  }

  /**
   * @brief Set result code of recent operation.
   * @details Updates internal result status. Stops bus on error.
   * @param lastResult Result code to set (default: SUCCESS).
   * @return The result code that was set.
   */
  inline ResultCodes setLastResult(
    ResultCodes lastResult = ResultCodes::SUCCESS)
  {
    if (lastResult != ResultCodes::SUCCESS)
    {
      setBusStop();
    }
    busStatus_.lastResult = lastResult;
    return busStatus_.lastResult;
  }

  /**
   * @brief Set two-wire bus clock frequency.
   * @details Updates bus clock speed. Takes effect at next I2C operation.
   * @param clockSpeed Clock frequency (100 kHz or 400 kHz).
   */
  inline void setBusClock(ClockSpeeds clockSpeed)
  {
    switch (clockSpeed)
    {
      case ClockSpeeds::CLOCK_100KHZ:
      case ClockSpeeds::CLOCK_400KHZ:
        busStatus_.clock = clockSpeed;
        break;
      default:
        busStatus_.clock = ClockSpeeds::CLOCK_100KHZ;
        break;
    };
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    setClock(busStatus_.clock);
#elif defined(PARTICLE)
    setSpeed(busStatus_.clock);
#endif
  }
  /// @}

  /// @name Getters
  /// @{
  /**
   * @brief Get result code of recent operation.
   * @return Result code.
   */
  inline ResultCodes getLastResult()
  {
    if (busStatus_.lastResult != ResultCodes::SUCCESS)
    {
      setBusStop();
    }
    return busStatus_.lastResult;
  }

  /**
   * @brief Get currently registered device address.
   * @return I2C device address.
   */
  inline uint8_t getAddress() { return busStatus_.address; }

  /**
   * @brief Get minimum valid I2C address.
   * @return Minimum address (0x00).
   */
  inline uint8_t getAddressMin() { return AddressRange::ADDRESS_MIN; }

  /**
   * @brief Get maximum valid I2C address.
   * @return Maximum address (0x7F).
   */
  inline uint8_t getAddressMax() { return AddressRange::ADDRESS_MAX; }

  /**
   * @brief Get minimum special purpose address.
   * @return Minimum special address (0x01).
   */
  inline uint8_t getAddressMinSpecial()
  {
    return AddressRange::ADDRESS_MIN_SPECIAL;
  }

  /**
   * @brief Get minimum usual device address.
   * @return Minimum usual address (0x03).
   */
  inline uint8_t getAddressMinUsual()
  {
    return AddressRange::ADDRESS_MIN_USUAL;
  }

  /**
   * @brief Get maximum usual device address.
   * @return Maximum usual address (0x77).
   */
  inline uint8_t getAddressMaxUsual()
  {
    return AddressRange::ADDRESS_MAX_USUAL;
  }

  /**
   * @brief Get SDA pin number.
   * @return SDA pin.
   */
  inline uint8_t getPinSDA() { return busStatus_.pinSDA; }

  /**
   * @brief Get SCL pin number.
   * @return SCL pin.
   */
  inline uint8_t getPinSCL() { return busStatus_.pinSCL; }

  /**
   * @brief Get recent command sent to the bus.
   * @return Last command value.
   */
  inline uint16_t getLastCommand() { return busStatus_.lastCommand; }

  /**
   * @brief Get bus clock frequency.
   * @return Clock speed in Hz.
   */
  inline ClockSpeeds getBusClock() { return busStatus_.clock; }

  /**
   * @brief Check if recent operation was successful.
   * @details Evaluates the internally stored result code.
   * @return True if the stored result code is SUCCESS, false otherwise.
   */
  inline bool isSuccess()
  {
    return busStatus_.lastResult == ResultCodes::SUCCESS;
  }

  /**
   * @brief Check if a given result code indicates success.
   * @details Evaluates the provided result code directly without modifying
   * internal state. Useful for evaluating return values of inline methods
   * directly in conditional expressions.
   * @param lastResult Result code to evaluate.
   * @return True if lastResult is SUCCESS, false otherwise.
   */
  inline bool isSuccess(ResultCodes lastResult)
  {
    return lastResult == ResultCodes::SUCCESS;
  }

  /**
   * @brief Check if recent operation resulted in an error.
   * @details Evaluates the internally stored result code.
   * @return True if the stored result code is not SUCCESS, false otherwise.
   */
  inline bool isError()
  {
    return !isSuccess();
  }

  /**
   * @brief Check if a given result code indicates an error.
   * @details Evaluates the provided result code directly without modifying
   * internal state. Useful for evaluating return values of inline methods
   * directly in conditional expressions.
   * @param lastResult Result code to evaluate.
   * @return True if lastResult is not SUCCESS, false otherwise.
   */
  inline bool isError(ResultCodes lastResult) { return !isSuccess(lastResult); }
  /// @}

  /**
   * @brief Convert error code to human-readable text.
   * @param location Optional location string for error context.
   * @return Formatted error message.
   */
  String getLastErrorTxt(String location = "");

  /**
   * @brief Set send operation delay.
   * @details Delays before subsequent send after completion of previous send.
   * @param delay Wait time in milliseconds.
   */
  inline void setDelaySend(uint32_t delay) { busStatus_.sendDelay = delay; }

  /**
   * @brief Get send operation delay.
   * @return Delay in milliseconds.
   */
  inline uint32_t getDelaySend() { return busStatus_.sendDelay; }

  /**
   * @brief Set receive operation delay.
   * @details Delays before subsequent receive after completion of previous
   * receive.
   * @param delay Wait time in milliseconds.
   */
  inline void setDelayReceive(uint32_t delay)
  {
    busStatus_.receiveDelay = delay;
  }

  /**
   * @brief Get receive operation delay.
   * @return Delay in milliseconds.
   */
  inline uint32_t getDelayReceive() { return busStatus_.receiveDelay; }

private:
  enum AddressRange : uint8_t
  {
    /// Minimal valid address
    ADDRESS_MIN = 0x00,
    /// Maximal valid address
    ADDRESS_MAX = 0x7F,
    /// Minimal special purposes address
    ADDRESS_MIN_SPECIAL = 0x01,
    /// Minimal usual address
    ADDRESS_MIN_USUAL = 0x03,
    /// Maximal usual address
    ADDRESS_MAX_USUAL = 0x77,
    /// General call address
    ADDRESS_GENCALL = 0x00,
  };

  enum GeneralCall : uint8_t
  {
    /// Software reset and write programmable part of slave address
    GENCALL_RESET = 0x06,
    /// Write programmable part of slave address only
    GENCALL_WRITE = 0x04,
  };

  enum DataStreamProcessing : uint8_t
  {
    /// Process data stream with the least significant byte first
    STREAM_DIR_LSB = 0,
    /// Process data stream with the most significant byte first
    STREAM_DIR_MSB = 1,
    /// Process only non-zero bytes from LSB - ignore zero MSB bytes
    STREAM_BYTES_VAL = 2,
    /// Process all bytes of data stream regardless of their value
    STREAM_BYTES_ALL = 3,
    /// I2C Buffer length adopted from parent library
    STREAM_BUFFER_LENGTH = BUFFER_LENGTH,
  };

  struct BusStatus
  {
    /// Result of a recent operation
    ResultCodes lastResult;
    /// Command code recently sent to two-wire bus
    uint16_t lastCommand;
    /// Address of the device on two-wire bus
    uint8_t address = 255;
    /// Clock frequency in Hz
    ClockSpeeds clock;
    /// Flag about releasing bus after end of transmission
    bool busStop;
    /// Pin for serial data
    uint8_t pinSDA;
    /// Pin for serial clock
    uint8_t pinSCL;
    /// Mode of data stream processing
    uint8_t streamDirection;
    /// Mode of data stream bytes processing
    uint8_t streamBytes;
    /// Waiting after each sent page
    uint32_t sendDelay = 0;
    /// Waiting after each received page
    uint32_t receiveDelay = 0;
    /// Recent bus transmission timestamp
    uint32_t transTimestamp = 0;
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    bool busEnabled; // Flag about bus initialization
#endif
  } busStatus_; /// Microcontroller status features

  /**
   * @brief Set recent command sent to bus.
   * @param lastCommand Command value.
   * @return The command value.
   */
  inline uint16_t setLastCommand(uint16_t lastCommand)
  {
    return busStatus_.lastCommand = lastCommand;
  }

  /**
   * @brief Buffer data word according to stream settings.
   * @details Updates buffer from index with data according to stream direction
   * and byte processing modes, and advances index.
   * @param dataBuffer Pointer to byte buffer to update.
   * @param dataIdx Reference to buffer index (will be incremented).
   * @param data Data word to buffer.
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
  /// @name Bus state management
  /// @{
  /**
   * @brief Set bus stop flag.
   * @details Controls STOP condition generation at end of transmission.
   * @param busStop True to generate STOP, false for repeated START.
   */
  inline void setBusStopFlag(bool busStop) { busStatus_.busStop = busStop; }

  /**
   * @brief Generate STOP condition at end of transmission.
   */
  inline void setBusStop() { setBusStopFlag(true); }

  /**
   * @brief Use repeated START instead of STOP condition.
   */
  inline void setBusRepeat() { setBusStopFlag(false); }

  /**
   * @brief Get current bus stop flag setting.
   * @return True if STOP will be generated, false if repeated START.
   */
  inline bool getBusStop() { return busStatus_.busStop; }
  /// @}

  /// @name Data stream direction management
  /// @{
  /**
   * @brief Get current data stream direction setting.
   * @return Stream direction mode.
   */
  inline uint8_t getStreamDir() { return busStatus_.streamDirection; }

  /**
   * @brief Set data stream direction to LSB first.
   */
  inline void setStreamDirLSB()
  {
    busStatus_.streamDirection = DataStreamProcessing::STREAM_DIR_LSB;
  }

  /**
   * @brief Set data stream direction to MSB first.
   */
  inline void setStreamDirMSB()
  {
    busStatus_.streamDirection = DataStreamProcessing::STREAM_DIR_MSB;
  }

  /**
   * @brief Set data stream direction to default (MSB first).
   */
  inline void setStreamDirDft() { setStreamDirMSB(); }
  /// @}

  /// @name Data stream bytes processing management
  /// @{
  /**
   * @brief Get current byte processing mode.
   * @return Byte processing mode.
   */
  inline uint8_t getStreamBytes() { return busStatus_.streamBytes; }

  /**
   * @brief Set byte processing to value-based (skip zero MSB).
   */
  inline void setStreamBytesVal()
  {
    busStatus_.streamBytes = DataStreamProcessing::STREAM_BYTES_VAL;
  }

  /**
   * @brief Set byte processing to all bytes.
   */
  inline void setStreamBytesAll()
  {
    busStatus_.streamBytes = DataStreamProcessing::STREAM_BYTES_ALL;
  }

  /**
   * @brief Set byte processing to default (value-based).
   */
  inline void setStreamBytesDft() { setStreamBytesVal(); }
  /// @}

  /// @name Timestamp management
  /// @{
  /**
   * @brief Set transmission timestamp.
   * @param timestamp Optional custom timestamp (default: current millis()).
   */
  inline void setTimestamp(uint32_t timestamp = millis())
  {
    busStatus_.transTimestamp = timestamp;
  }

  /**
   * @brief Get recent transmission timestamp.
   * @return Timestamp in milliseconds.
   */
  inline uint32_t getTimestamp() { return busStatus_.transTimestamp; }

  /**
   * @brief Wait until send delay expires.
   */
  inline void waitTimestampSend()
  {
    while (millis() - busStatus_.transTimestamp < getDelaySend())
      ;
  }

  /**
   * @brief Wait until receive delay expires.
   */
  inline void waitTimestampReceive()
  {
    while (millis() - busStatus_.transTimestamp < getDelayReceive())
      ;
  }
  /// @}

  /**
   * @brief Wait for specified duration.
   * @details Blocking delay using polling.
   * @param delay Wait time in milliseconds.
   */
  inline void wait(uint32_t delay)
  {
    uint32_t timestamp = millis();
    while (millis() - timestamp < delay)
    {
      ;
    }
  }

  /**
   * @brief Initialize two-wire bus if not already initialized.
   * @details Starts bus and sets platform-specific configuration.
   */
  inline void initBus()
  {
    setLastResult();
#if defined(__AVR__)
    if (!busStatus_.busEnabled)
    {
      Wire.begin();
      busStatus_.busEnabled = true;
    }
#elif defined(ESP8266) || defined(ESP32)
    if (!busStatus_.busEnabled)
    {
      Wire.begin(busStatus_.pinSDA, busStatus_.pinSCL);
      busStatus_.busEnabled = true;
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
