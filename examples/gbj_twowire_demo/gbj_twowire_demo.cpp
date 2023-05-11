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
  Serial.println(device.getLastErrorTxt(location));
  Serial.println("---");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("---");
  // Test constructor success
  if (device.isError())
  {
    errorHandler("Constructor");
    return;
  }
  // Initial two-wire bus
  if (device.isError(device.begin()))
  {
    errorHandler("Begin");
    return;
  }
  // Set and test address
  if (device.isError(device.setAddress(ADDRESS_DEVICE)))
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
