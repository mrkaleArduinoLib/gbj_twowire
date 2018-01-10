#include "gbj_twowire.h"


// Constructor
gbj_twowire::gbj_twowire(){}


// Destructor
gbj_twowire::~gbj_twowire()
{
  release();
}


void gbj_twowire::release()
{
  end();
#if defined(__AVR__)
  _busEnabled = false;
#endif
}


uint8_t gbj_twowire::busWrite(uint16_t data)
{
  uint8_t countByte = 0;
  uint8_t dataByte;
  // Write MSB of int if not zero
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
  busWrite(command);
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
bool gbj_twowire::setBusStop(bool busStop)
{
  _busStop = busStop;
  return getBusStop();
}


uint8_t gbj_twowire::setAddress(uint8_t address)
{
  initLastResult();
  // Invalid address
  if (address < GBJ_TWOWIRE_ADDRESS_MIN || address > GBJ_TWOWIRE_ADDRESS_MAX)
  {
    return setLastResult(GBJ_TWOWIRE_ERR_ADDRESS);
  }
  // No address change
  if (address == getAddress()) return getLastResult();
  // Set changed address
  _address = address;
  if (!getBusStop()) end();
  initBus();
  beginTransmission(getAddress());
  setLastResult(endTransmission(getBusStop()));
  return getLastResult();
}


uint8_t gbj_twowire::setLastResult(uint8_t lastResult)
{
  _lastResult = lastResult;
  return getLastResult();
}


void gbj_twowire::initLastResult()
{
  _lastResult = GBJ_TWOWIRE_SUCCESS;
}


//------------------------------------------------------------------------------
// Getters
//------------------------------------------------------------------------------
uint8_t gbj_twowire::getLastResult()  { return _lastResult; }
uint8_t gbj_twowire::getAddress()     { return _address; }
bool    gbj_twowire::getBusStop()     { return _busStop; }
bool    gbj_twowire::isSuccess()      { return _lastResult == GBJ_TWOWIRE_SUCCESS; }
bool    gbj_twowire::isError()        { return !isSuccess(); }


//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------


// Wait for delay period expiry
void gbj_twowire::wait(uint32_t delay)
{
  uint32_t timestamp = millis();
  while (millis() - timestamp < delay);
}


// Initialize two wire bus
void gbj_twowire::initBus()
{
  initLastResult();
#if defined(__AVR__)
  if (!_busEnabled)
  {
    // setClock(CLOCK_SPEED_100KHZ);
    // setClock(CLOCK_SPEED_400KHZ);
    begin();
    _busEnabled = true;
  }
#elif defined(PARTICLE)
  if (!isEnabled())
  {
    // setSpeed(CLOCK_SPEED_100KHZ);
    // setSpeed(CLOCK_SPEED_400KHZ);
    begin();
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
