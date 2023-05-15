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

String gbj_twowire::getLastErrorTxt(String location)
{
  String result = "";
  result += location.length() ? location + "::" : "";
  // Ignore success code
  if (_busStatus.lastResult == ResultCodes::SUCCESS)
  {
    result += "SUCCESS";
    return result;
  }
  result += "Error: ";
  switch (_busStatus.lastResult)
  {
    // General
    case ResultCodes::ERROR_ADDRESS:
      result += "ERROR_ADDRESS";
      break;

    case ResultCodes::ERROR_PINS:
      result += "ERROR_PINS";
      break;

    case ResultCodes::ERROR_RCV_DATA:
      result += "ERROR_RCV_DATA";
      break;

    case ResultCodes::ERROR_POSITION:
      result += "ERROR_POSITION";
      break;

    case ResultCodes::ERROR_DEVICE:
      result += "ERROR_DEVICE";
      break;

    case ResultCodes::ERROR_RESET:
      result += "ERROR_RESET";
      break;

    case ResultCodes::ERROR_FIRMWARE:
      result += "ERROR_FIRMWARE";
      break;

    case ResultCodes::ERROR_SN:
      result += "ERROR_SN";
      break;

    case ResultCodes::ERROR_MEASURE:
      result += "ERROR_MEASURE";
      break;

    case ResultCodes::ERROR_REGISTER:
      result += "ERROR_REGISTER";
      break;

      // Arduino, Esspressif specific
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    case ResultCodes::ERROR_BUFFER:
      result += "ERROR_BUFFER";
      break;

    case ResultCodes::ERROR_NACK_DATA:
      result += "ERROR_NACK_DATA";
      break;

    case ResultCodes::ERROR_NACK_OTHER:
      result += "ERROR_NACK_OTHER";
      break;

      // Particle specific
#elif defined(PARTICLE)
    case ResultCodes::ERROR_BUSY:
      result += "ERROR_BUSY";
      break;

    case ResultCodes::ERROR_END:
      result += "ERROR_END";
      break;

    case ResultCodes::ERROR_TRANSFER:
      result += "ERROR_TRANSFER";
      break;

    case ResultCodes::ERROR_TIMEOUT:
      result += "ERROR_TIMEOUT";
      break;
#endif
    default:
      result += "ERROR_UKNOWN";
      break;
  }
  result += " (" + String(_busStatus.lastResult) + ")";
  // Last command
  if (_busStatus.lastCommand)
  {
    result += ", Command: 0x";
    result += String(_busStatus.lastCommand, HEX);
  }
  return result;
}
