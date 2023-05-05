/*
  NAME:
  Demonstration usage of gbjTwoWire library.

  DESCRIPTION:
  The sketch sets address of a device and checks if it is correct.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the MIT License (MIT).

  CREDENTIALS:
  Author: Libor Gabaj
*/
#include "gbj_twowire.h"

// Comment/Uncomment/Update the good and wrong value for experimenting
const byte ADDRESS_DEVICE = 0x23; // Good for BH1750FVI
// const byte ADDRESS_DEVICE = 0x24;  // Wrong
gbj_twowire device = gbj_twowire();
// gbj_twowire device = gbj_twowire(gbj_twowire::ClockSpeed::CLOCK_400KHZ);
// gbj_twowire device = gbj_twowire(gbj_twowire::ClockSpeed::CLOCK_100KHZ);
// gbj_twowire device = gbj_twowire(gbj_twowire::ClockSpeed::CLOCK_100KHZ, D2,
// D1);

void errorHandler(String location)
{
  if (device.isSuccess())
  {
    return;
  }
  Serial.print(location);
  Serial.print(" - Error: ");
  Serial.print(device.getLastResult());
  Serial.print(" - ");
  switch (device.getLastResult())
  {
    // General
    case gbj_twowire::ResultCodes::ERROR_ADDRESS:
      Serial.println("ERROR_ADDRESS");
      break;

    case gbj_twowire::ResultCodes::ERROR_PINS:
      Serial.println("ERROR_PINS");
      break;

      // Arduino, Esspressif specific
#if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
    case gbj_twowire::ResultCodes::ERROR_BUFFER:
      Serial.println("ERROR_BUFFER");
      break;

    case gbj_twowire::ResultCodes::ERROR_NACK_DATA:
      Serial.println("ERROR_NACK_DATA");
      break;

    case gbj_twowire::ResultCodes::ERROR_NACK_OTHER:
      Serial.println("ERROR_NACK_OTHER");
      break;

      // Particle specific
#elif defined(PARTICLE)
    case gbj_twowire::ResultCodes::ERROR_BUSY:
      Serial.println("ERROR_BUSY");
      break;

    case gbj_twowire::ResultCodes::ERROR_END:
      Serial.println("ERROR_END");
      break;

    case gbj_twowire::ResultCodes::ERROR_TRANSFER:
      Serial.println("ERROR_TRANSFER");
      break;

    case gbj_twowire::ResultCodes::ERROR_TIMEOUT:
      Serial.println("ERROR_TIMEOUT");
      break;
#endif

    default:
      Serial.println("Uknown error");
      break;
  }
}

void setup()
{
  Serial.begin(9600);
  // Test constructor success
  if (device.isError())
  {
    errorHandler("Constructor");
    return;
  }
  // Initial two-wire bus
  if (device.begin())
  {
    errorHandler("Begin");
    return;
  }
  // Set and test address
  device.setAddress(ADDRESS_DEVICE);
  if (device.isError())
  {
    errorHandler("Address");
    return;
  }
  Serial.print("Address: 0x");
  Serial.println(device.getAddress(), HEX);
  Serial.print("Bus Clock: ");
  Serial.print(device.getBusClock() / 1000);
  Serial.println(" kHz");
  Serial.print("Pin SDA: ");
  Serial.println(device.getPinSDA());
  Serial.print("Pin SCL: ");
  Serial.println(device.getPinSCL());
  Serial.println("---");
}

void loop() {}
