#include "gbj_twowire.h"

gbj_twowire::ResultCodes gbj_twowire::busSendStream(uint8_t *dataBuffer,
                                                    uint16_t dataLen,
                                                    bool dataReverse)
{
  bool origBusStop = getBusStop();
  setLastResult();
  setBusRepeat();
  if (dataReverse)
  {
    dataBuffer += dataLen;
    dataBuffer--;
  }
  waitTimestampSend();
  while (dataLen)
  {
    uint8_t pageLen = BUFFER_LENGTH;
    beginTransmission(getAddress());
    while (pageLen > 0 && dataLen > 0)
    {
      if (dataReverse)
      {
        write(*dataBuffer--);
      }
      else
      {
        write(*dataBuffer++);
      }
      pageLen--;
      dataLen--;
    }
    // Return original flag at last page
    if (dataLen == 0)
    {
      setBusStopFlag(origBusStop);
    }
    if (setLastResult(static_cast<ResultCodes>(endTransmission(getBusStop()))))
    {
      return getLastResult();
    }
  }
  setTimestampSend();
  setBusStopFlag(origBusStop);
  return getLastResult();
}

gbj_twowire::ResultCodes gbj_twowire::busSendStreamPrefixed(uint8_t *dataBuffer,
                                                            uint16_t dataLen,
                                                            bool dataReverse,
                                                            uint8_t *prfxBuffer,
                                                            uint16_t prfxLen,
                                                            bool prfxReverse,
                                                            bool prfxOnetime)
{
  bool origBusStop = getBusStop();
  bool prfxExec = true;
  setLastResult();
  setBusRepeat();
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
  waitTimestampSend();
  while (dataLen)
  {
    uint8_t pageLen = BUFFER_LENGTH;
    beginTransmission(getAddress());
    // Injected prefix stream in every page
    if (prfxExec)
    {
      uint16_t prfxLenPage = prfxLen;
      uint8_t *prfxBufferPage = prfxBuffer;
      while (pageLen > 0 && prfxLenPage > 0)
      {
        if (prfxReverse)
        {
          write(*prfxBufferPage--);
        }
        else
        {
          write(*prfxBufferPage++);
        };
        pageLen--;
        prfxLenPage--;
      }
      if (prfxOnetime)
      {
        prfxExec = false;
      }
    }
    // Main data stream
    while (pageLen > 0 && dataLen > 0)
    {
      if (dataReverse)
      {
        write(*dataBuffer--);
      }
      else
      {
        write(*dataBuffer++);
      }
      pageLen--;
      dataLen--;
    }
    // Return original flag at last page
    if (dataLen == 0)
    {
      setBusStopFlag(origBusStop);
    }
    if (setLastResult(static_cast<ResultCodes>(endTransmission(getBusStop()))))
    {
      return getLastResult();
    }
  }
  setTimestampSend();
  setBusStopFlag(origBusStop);
  return getLastResult();
}

gbj_twowire::ResultCodes gbj_twowire::busReceive(uint8_t *dataBuffer,
                                                 uint16_t dataLen)
{
  bool origBusStop = getBusStop();
  setLastResult();
  setBusRepeat();
  waitTimestampReceive();
  while (dataLen)
  {
    uint8_t pageLen = min(dataLen, BUFFER_LENGTH);
    // Return original flag before last page
    if (pageLen >= dataLen)
    {
      setBusStopFlag(origBusStop);
    }
    if (requestFrom(getAddress(), pageLen, (uint8_t)getBusStop()) > 0 &&
        available() >= pageLen)
    {
      for (uint8_t i = 0; i < pageLen; i++)
      {
        *dataBuffer++ = read();
      }
    }
    else
    {
      return setLastResult(ERROR_RCV_DATA);
    }
    dataLen -= pageLen;
  }
  setTimestampReceive();
  setBusStopFlag(origBusStop);
  return getLastResult();
}

gbj_twowire::ResultCodes gbj_twowire::busReceive(uint16_t command,
                                                 uint8_t *dataBuffer,
                                                 uint16_t dataLen)
{
  bool origBusStop = getBusStop();
  setBusRepeat();
  if (busSend(setLastCommand(command)))
  {
    return getLastResult();
  }
  setBusStopFlag(origBusStop);
  if (busReceive(dataBuffer, dataLen))
  {
    return getLastResult();
  }
  return getLastResult();
}
