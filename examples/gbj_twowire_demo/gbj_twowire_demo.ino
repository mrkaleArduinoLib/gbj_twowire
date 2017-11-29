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

// Update for good and wrong value to experiment
// const byte ADDRESS_DEVICE = 0x3C;   // Good
const byte ADDRESS_DEVICE = 0x3D;   // Wrong
gbj_twowire Device = gbj_twowire();

void setup()
{
  Serial.begin(9600);
  Device.initLastResult();
  Device.setAddress(ADDRESS_DEVICE);
  if (Device.getLastResult() == GBJ_TWOWIRE_SUCCESS)
  {
    Serial.print("Success: ");
    Serial.println(Device.getLastResult());
  }
  else
  {
    Serial.print("Error: ");
    Serial.print(Device.getLastResult());
    Serial.print(" - ");
    switch (Device.getLastResult())
    {
      case GBJ_TWOWIRE_ERR_NACK_ADDR:
        Serial.println("Bad address");
        break;

      case GBJ_TWOWIRE_ERR_OTHER:
        Serial.println("Other error");
        break;

      default:
        Serial.println("Uknown error");
        break;
    }
  }
}

void loop() {}
