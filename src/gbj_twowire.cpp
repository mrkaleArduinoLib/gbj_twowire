#include "gbj_twowire.h"
const String gbj_twowire::VERSION = "GBJ_TWOWIRE 1.0.0";


// Constructor
gbj_twowire::gbj_twowire(uint32_t clockSpeed, bool busStop, uint8_t pinSDA, uint8_t pinSCL)
{
  setPins(pinSDA, pinSCL);
  setBusClock(clockSpeed);
  setBusStop(busStop);
}


// Destructor
gbj_twowire::~gbj_twowire()
{
  release();
}


uint8_t gbj_twowire::begin()
{
  initLastResult();
#if defined(ESP8266) || defined(ESP32)
  // Check pin duplicity
  if (_status.pinSDA == _status.pinSCL) return setLastResult(ERROR_PINS);
#endif
  return getLastResult();
}


void gbj_twowire::release()
{
#if defined(__AVR__) || defined(PARTICLE)
  end();
#endif
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  _status.busEnabled = false;
#endif
}


uint8_t gbj_twowire::busWrite(uint16_t data)
{
  uint8_t countByte = 0;
  uint8_t dataByte;
  // Write MSB of a word if not zero
  dataByte = (uint8_t) (data >> 8);
  if (dataByte > 0x00)
  {
    countByte += platformWrite(dataByte);
  }
  // Write LSB allways
  dataByte = (uint8_t) (data & 0x00FF);
  countByte += platformWrite(dataByte);
  return countByte;
}


uint8_t gbj_twowire::busSend(uint16_t command, uint16_t data)
{
  initBus();
  beginTransmission(getAddress());
  busWrite(setLastCommand(command));
  busWrite(data);
  if (setLastResult(endTransmission(getBusStop()))) return getLastResult();
  return getLastResult();
}


uint8_t gbj_twowire::busSend(uint16_t data)
{
  initBus();
  beginTransmission(getAddress());
  busWrite(data);
  if (setLastResult(endTransmission(getBusStop()))) return getLastResult();
  return getLastResult();
}


uint8_t gbj_twowire::busRead()
{
  #if ARDUINO >= 100
    return read();
  #else
    return receive();
  #endif
}


uint8_t gbj_twowire::busReceive(uint8_t dataArray[], uint8_t bytes, uint8_t start)
{
  initBus();
  beginTransmission(getAddress());
  if (requestFrom(getAddress(), bytes, (uint8_t) getBusStop()) > 0 \
  && available() >= bytes)
  {
    for (uint8_t i = 0; i < bytes; i++)
    {
      dataArray[i + start] = busRead();
    }
  }
  setLastResult(endTransmission(getBusStop()));
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
  _status.address = address;
#if defined(__AVR__) || defined(PARTICLE)
  if (!getBusStop()) end();
#endif
  initBus();
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
      _status.clock = clockSpeed;
      break;
    default:
      _status.clock = CLOCK_100KHZ;
      break;
  };
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
  setClock(_status.clock);
#elif defined(PARTICLE)
  setSpeed(_status.clock);
#endif
}


uint8_t gbj_twowire::setPins(uint8_t pinSDA, uint8_t pinSCL)
{
  _status.pinSDA = pinSDA;
  _status.pinSCL = pinSCL;
  initLastResult();
#if defined(ESP8266) || defined(ESP32)
  // Check pin duplicity
  if (_status.pinSDA == _status.pinSCL) return setLastResult(ERROR_PINS);
#endif
  return getLastResult();
}


//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------
// Wait for delay period expiry
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
  if (!_status.busEnabled)
  {
    setBusClock(_status.clock);
    Wire.begin();
    _status.busEnabled = true;
  }
#elif defined(ESP8266) || defined(ESP32)
  if (!_status.busEnabled)
  {
    setBusClock(_status.clock);
    Wire.begin(_status.pinSDA, _status.pinSCL);
    _status.busEnabled = true;
  }
#elif defined(PARTICLE)
  if (!isEnabled())
  {
    setBusClock(_status.clock);
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
