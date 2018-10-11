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
  Device.setAddress(ADDRESS_DEVICE);
  if (Device.isError())
  {
    errorHandler();
    return;
  }
  Serial.print("Success: ");
  Serial.println(Device.getLastResult());
  Serial.print("Bus Clock (Hz): ");
  Serial.println(Device.getBusClock());
  Serial.print("Bus Stop: ");
  Serial.println(Device.getBusStop() ? "Yes" : "No");
}

void loop() {}
