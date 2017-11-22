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
  _address = address;
  return checkAddress();
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
// Private methods
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


// Check two wire bus address
uint8_t gbj_twowire::checkAddress()
{
  initLastResult();
  if (_address > 0)
  {
    // Release bus at keep alive
    if (!getBusStop()) end();
    initBus();
    beginTransmission(_address);
    setLastResult(endTransmission(getBusStop()));
  }
  return getLastResult();
}
