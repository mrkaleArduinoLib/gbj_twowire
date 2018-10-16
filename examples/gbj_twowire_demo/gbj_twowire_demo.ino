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
#define SKETCH "GBJ_TWOWIRE_DEMO 1.0.0"

#include "gbj_twowire.h"

// Comment/Uncomment/Update for good and wrong value to experiment
const byte ADDRESS_DEVICE = 0x23;  // Good
// const byte ADDRESS_DEVICE = 0x24;  // Wrong
gbj_twowire Device = gbj_twowire();
// gbj_twowire Device = gbj_twowire(gbj_twowire::CLOCK_400KHZ);
// gbj_twowire Device = gbj_twowire(gbj_twowire::CLOCK_100KHZ, false);
// gbj_twowire Device = gbj_twowire(gbj_twowire::CLOCK_100KHZ, true, D2, D1);


void errorHandler()
{
  if (Device.isSuccess()) return;
  Serial.print("Error: ");
  Serial.print(Device.getLastResult());
  Serial.print(" - ");
  switch (Device.getLastResult())
  {
    case gbj_twowire::ERROR_ADDRESS:
      Serial.println("Bad address");
      break;

    case gbj_twowire::ERROR_PINS:
      Serial.println("Bad pins");
      break;

    case gbj_twowire::ERROR_NACK_OTHER:
      Serial.println("Other error");
      break;

    default:
      Serial.println("Uknown error");
      break;
  }
}


void setup()
{
  Serial.begin(9600);
  // Test constructor success
  if (Device.isError())
  {
    errorHandler();
    return;
  }
  // Set and test address
  Device.setAddress(ADDRESS_DEVICE);
  if (Device.isError())
  {
    errorHandler();
    return;
  }
  Serial.print("Success: ");
  Serial.println(Device.getLastResult());
  Serial.print("Bus Clock: ");
  Serial.print(Device.getBusClock() / 1000);
  Serial.println(" kHz");
  Serial.print("Bus Stop: ");
  Serial.println(Device.getBusStop() ? "Yes" : "No");
  Serial.print("Pin SDA: ");
  Serial.println(Device.getPinSDA());
  Serial.print("Pin SCL: ");
  Serial.println(Device.getPinSCL());
}

void loop() {}
