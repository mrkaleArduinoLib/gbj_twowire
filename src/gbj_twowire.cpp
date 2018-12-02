#include "gbj_twowire.h"
const String gbj_twowire::VERSION = "GBJ_TWOWIRE 1.0.0";


// Constructor
gbj_twowire::gbj_twowire(uint32_t clockSpeed, bool busStop, uint8_t pinSDA, uint8_t pinSCL)
{
  setBusClock(clockSpeed);
  setBusStop(busStop);
  setPins(pinSDA, pinSCL);
}


// Destructor
gbj_twowire::~gbj_twowire()
{
  release();
}


uint8_t gbj_twowire::begin()
{
  initBus();
#if defined(ESP8266) || defined(ESP32)
  // Check pin duplicity
  if (_busStatus.pinSDA == _busStatus.pinSCL) return setLastResult(ERROR_PINS);
#endif
  return getLastResult();
}


void gbj_twowire::release()
{
#if defined(__AVR__) || defined(PARTICLE)
  end();
#endif
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  _busStatus.busEnabled = false;
#endif
}


uint8_t gbj_twowire::busSendStream(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse)
{
  initLastResult();
  if (dataReverse)
  {
    dataBuffer += dataLen;
    dataBuffer--;
  }
  while (dataLen)
  {
    uint8_t pageLen = BUFFER_LENGTH;
    waitTimestampSend();
    beginTransmission(getAddress());
    while (pageLen > 0 && dataLen > 0)
    {
      if (dataReverse) platformWrite(*dataBuffer--);
      else             platformWrite(*dataBuffer++);
      pageLen--;
      dataLen--;
    }
    if (setLastResult(endTransmission(getBusStop()))) return getLastResult();
    setTimestampSend();
  }
  return getLastResult();
}


uint8_t gbj_twowire::busSendStreamPrefixed(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse, \
  uint8_t *prfxBuffer, uint16_t prfxLen, bool prfxReverse, bool prfxOnetime)
{
  bool prfxExec = true;
  initLastResult();
  if (dataReverse)
  {
    dataBuffer += dataLen;
    dataBuffer--;
  }
  if (prfxReverse)
  {
    prfxBuffer += prfxLen;
    prfxBuffer--;
  }
  while (dataLen)
  {
    uint8_t pageLen = BUFFER_LENGTH;
    waitTimestampSend();
    beginTransmission(getAddress());
    // Injected prefix stream in every page
    if (prfxExec)
    {
      uint16_t prfxLenPage = prfxLen;
      uint8_t *prfxBufferPage = prfxBuffer;
      while (pageLen > 0 && prfxLenPage > 0)
      {
        if (prfxReverse) platformWrite(*prfxBufferPage--);
        else             platformWrite(*prfxBufferPage++);
        pageLen--;
        prfxLenPage--;
      }
      if (prfxOnetime) prfxExec = false;
    }
    // Main data stream
    while (pageLen > 0 && dataLen > 0)
    {
      if (dataReverse) platformWrite(*dataBuffer--);
      else             platformWrite(*dataBuffer++);
      pageLen--;
      dataLen--;
    }
    if (setLastResult(endTransmission(getBusStop()))) return getLastResult();
    setTimestampSend();
  }
  return getLastResult();
}


uint8_t gbj_twowire::busSend(uint16_t data)
{
  uint8_t *ptrData = (uint8_t*)&data;
  if (data < 0x100) return busSendStream(ptrData, 1);
  return busSendStream(ptrData, sizeof(data), true);
}


uint8_t gbj_twowire::busSend(uint16_t command, uint16_t data)
{
  if (busSend(setLastCommand(command))) return getLastResult();
  return busSend(data);
}


uint8_t gbj_twowire::busRead()
{
  #if ARDUINO >= 100
    return read();
  #else
    return receive();
  #endif
}


uint8_t gbj_twowire::busReceive(uint8_t *dataBuffer, uint16_t dataLen)
{
  initLastResult();
  while (dataLen)
  {
    uint8_t pageLen = min(dataLen, BUFFER_LENGTH);
    waitTimestampReceive();
    beginTransmission(getAddress());
    if (requestFrom(getAddress(), pageLen, (uint8_t) getBusStop()) > 0 \
    && available() >= pageLen)
    {
      for (uint8_t i = 0; i < pageLen; i++)
      {
        *dataBuffer++ = busRead();
      }
    }
    if (setLastResult(endTransmission(getBusStop()))) return getLastResult();
    setTimestampReceive();
    dataLen -= pageLen;
  }
  return getLastResult();
}


uint8_t gbj_twowire::busGeneralReset()
{
  initBus();
  beginTransmission(ADDRESS_GENCALL);
  platformWrite(GENCALL_RESET);
  if (setLastResult(endTransmission(getBusStop()))) return getLastResult();
  setTimestampSend();
  return getLastResult();
}


//------------------------------------------------------------------------------
// Setters
//------------------------------------------------------------------------------
uint8_t gbj_twowire::setAddress(uint8_t address)
{
  initLastResult();
  // Invalid address
  if (address < ADDRESS_MIN || address > ADDRESS_MAX)
  {
    return setLastResult(ERROR_ADDRESS);
  }
  // No address change
  if (address == getAddress()) return getLastResult();
  // Set changed address
  _busStatus.address = address;
#if defined(__AVR__) || defined(PARTICLE)
  if (!getBusStop()) end();
#endif
  beginTransmission(getAddress());
  setLastResult(endTransmission(getBusStop()));
  return getLastResult();
}


void gbj_twowire::setBusClock(uint32_t clockSpeed)
{
  switch (clockSpeed)
  {
    case CLOCK_100KHZ:
    case CLOCK_400KHZ:
      _busStatus.clock = clockSpeed;
      break;
    default:
      _busStatus.clock = CLOCK_100KHZ;
      break;
  };
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  setClock(_busStatus.clock);
#elif defined(PARTICLE)
  setSpeed(_busStatus.clock);
#endif
}


uint8_t gbj_twowire::setPins(uint8_t pinSDA, uint8_t pinSCL)
{
  _busStatus.pinSDA = pinSDA;
  _busStatus.pinSCL = pinSCL;
  initLastResult();
#if defined(ESP8266) || defined(ESP32)
  // Check pin duplicity
  if (_busStatus.pinSDA == _busStatus.pinSCL) return setLastResult(ERROR_PINS);
#endif
  return getLastResult();
}


//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------
void gbj_twowire::wait(uint32_t delay)
{
  uint32_t timestamp = millis();
  while (millis() - timestamp < delay);
}


// Initialize two-wire bus
void gbj_twowire::initBus()
{
  initLastResult();
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
}


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
uint8_t gbj_twowire::platformWrite(uint8_t data)
{
  #if ARDUINO >= 100
    return write(data);
  #else
    return send(data);
  #endif
}
