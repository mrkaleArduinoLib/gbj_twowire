<a id="library"></a>
# gbjTwoWire
The library embraces and provides common methods used at every application working with sensor on two-wire (I2C) bus.
- Library specifies (inherits from) the system `TwoWire` library.
- The class from the library is not intended to be used directly in a sketch, just as a parent class for specific sensor libraries.
- Library respects two-wire buffer length (32 byte) at communication on the bus by paging, so that it splits long byte streams into separate transactions.
- Library implements extended error handling.
- Library provides some general system methods implemented differently for various platforms, especially for ones with hardware two-wire bus implementation (Arduino, Particle - Photon, Electron...) and for ones with software (bit-banged) defined two-wire bus (Espressif - ESP8266, ESP32).
- Library initiates the two-wire bus at default speed (serial clock) **100 kHz**, but bus speed can be change dynamically.
- It is expected, that initialization of a device and setting its parameters is provided in a subclass inherited from the class defined in this library in the method `begin`, which is not defined in the library.
- Library does not use the built-in function `delay()` at waiting for some actions, e.g., waking up sensor from power down mode, but instead of it uses the own implementation of the `wait()` function based on system `millis()` function.
- Library allows address range 0x00 ~ 0x7F.
- Library utilizes the general call on the two-wire bus as well.


<a id="dependency"></a>
## Dependency

#### Particle platform
- **Particle.h**: Includes alternative (C++) data type definitions.

#### Arduino platform
- **Arduino.h**: Main include file for the Arduino SDK version greater or equal to 100.
- **WProgram.h**: Main include file for the Arduino SDK version less than 100.
- **inttypes.h**: Integer type conversions. This header file includes the exact-width integer definitions and extends them with additional facilities provided by the implementation.
- **TwoWire**: I2C system library loaded from the file `Wire.h`.

#### Espressif platform
- **Arduino.h**: Main include file for the Arduino SDK version greater or equal to 100.
- **TwoWire**: I2C system library loaded from the file `Wire.h`.


<a id="constants"></a>
## Constants
- **gbj\_twowire::VERSION**: Name and semantic version of the library.
- **gbj\_twowire::CLOCK\_100KHZ**: Bus clock speed 100 kHz.
- **gbj\_twowire::CLOCK\_400KHZ**: Bus clock speed 400 kHz.
- **gbj\_twowire::SUCCESS**: Result code for successful processing.

### Arduino and Espressif errors
- **gbj\_twowire::ERROR\_BUFFER**: Data too long to fit in transmit buffer.
- **gbj\_twowire::ERROR\_NACK\_ADDR**: Received NACK on transmit of address.
- **gbj\_twowire::ERROR\_NACK\_DATA**: Received NACK on transmit of data.
- **gbj\_twowire::ERROR\_NACK\_OTHER**:  Other error.

### Particle error codes
- **gbj\_twowire::ERROR\_BUSY**: Busy timeout upon entering `endTransmission()`.
- **gbj\_twowire::ERROR\_START**: START bit generation timeout.
- **gbj\_twowire::ERROR\_END**: End of address transmission timeout.
- **gbj\_twowire::ERROR\_TRANSFER**: Data byte transfer timeout.
- **gbj\_twowire::ERROR\_TIMEOUT**: Data byte transfer succeeded, busy timeout immediately after.

### Common errors
- **gbj\_twowire::ERROR\_ADDRESS**: Platform specific error code at bad address.
- **gbj\_twowire::ERROR\_PINS**: For software defined two-wire bus platforms at bad defined bus pins (GPIOs), usually both are the same.


<a id="interface"></a>
## Interface
- [gbj_twowire()](#gbj_twowire)
- [~gbj_twowire()](#gbj_twowire)
- [begin()](#begin)
- [release()](#release)
- [busSendStream()](#busSendStream)
- [busSendStreamPrefixed()](#busSendStreamPrefixed)
- [busSend()](#busSend)
- [busRead()](#busRead)
- [busReceive()](#busReceive)
- [busGeneralReset()](#busGeneralReset)

#### Setters
- [setLastResult()](#setLastResult)
- [setAddress()](#setAddress)
- [setBusStop()](#setBusStop)
- [setBusClock()](#setBusClock)
- [setPins()](#setPins)
- [initLastResult()](#initLastResult)

#### Getters
- [getLastResult()](#getLastResult)
- [getLastCommand()](#getLastResult)
- [getAddress()](#getAddress)
- [getAddressMin()](#getAddressLimits)
- [getAddressMinSpecial()](#getAddressLimits)
- [getAddressMinUsual()](#getAddressLimits)
- [getAddressMax()](#getAddressLimits)
- [getAddressMaxUsual()](#getAddressLimits)
- [getBusStop()](#getBusStop)
- [getBusClock()](#getBusClock)
- [getPinSDA()](#getPins)
- [getPinSCL()](#getPins)
- [isSuccess()](#isSuccess)
- [isError()](#isError)

#### Protected
- [setDelaySend()](#setDelaySend)
- [getDelaySend()](#getDelaySend)
- [wait()](#wait)
- [initBus()](#initBus)


<a id="gbj_twowire"></a>
## gbj_twowire()
#### Description
Destructor `~gbj_twowire()` just releases the two-wire bus.
Constructor `gbj_twowire()` creates the class instance object and sets some bus parameters that are usually constant in a sketch. On the other hand, constructor parameters can be changed individually later in a sketch, if it is needed to change them dynamically.
- If subclass inherited from this class does not need special constructor or destructor, that class does not need to define constructor and destructor whatsoever.

#### Syntax
    gbj_twowire(uint32_t clockSpeed, bool busStop, uint8_t pinSDA, uint8_t pinSCL);

#### Parameters
<a id="prm_busClock"></a>
- **clockSpeed**: Initial two-wire bus clock frequency in Hertz. If the clock is not from enumeration, it fallbacks to 100 kHz.
  - *Valid values*: gbj\_twowire::CLOCK\_100KHZ, gbj\_twowire::CLOCK\_400KHZ
  - *Default value*: gbj\_twowire::CLOCK\_100KHZ


<a id="prm_busStop"></a>
- **busStop**: Logical flag about releasing bus after end of transmission.
  - *Valid values*: true, false
    - **true**: Releases the bus after data transmission and enables other master devices to control the bus.
    - **false**: Keeps connection to the bus and enables to begin further data transmission immediately.
  - *Default value*: true


<a id="prm_pinSDA"></a>
- **pinSDA**: Microcontroller's pin for serial data. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms for communication on the bus. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  - *Valid values*: positive integer
  - *Default value*: 4 (GPIO4, D2)


<a id="prm_pinSCL"></a>
- **pinSCL**: Microcontroller's pin for serial clock. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  - *Valid values*: positive integer
  - *Default value*: 5 (GPIO5, D1)

#### Returns
Object performing the extended two-wire bus management.
The constructor cannot return [a result or error code](#constants) directly, however, it stores them in the instance object. The result can be tested in the operational code with the method [getLastResult()](#getLastResult), [isError()](#isError), or [isSuccess()](#isSuccess).

[Back to interface](#interface)


<a id="begin"></a>
## begin()
#### Description
The method checks the microcontroller's pins defined in the [constructor](gbj_twowire) for software defined two-wire bus platforms.
- The method checks whether pins set by constructor are not mutually equal.
- For hardware two-wire bus platforms the method ignores defined or default pins.

#### Syntax
	uint8_t begin();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

[Back to interface](#interface)


<a id="release"></a>
## release()
#### Description
The method releases the two-wire bus, so that pins used by it are available for general purpose I/O.

#### Syntax
    void release();

#### Parameters
None

#### Returns
None

[Back to interface](#interface)


<a id="initLastResult"></a>
## initLastResult()
#### Description
The method sets internal status of recent processing on the two-wire bus to success with value of macro [gbj\_twowire::SUCCESS](#constants). It is usually called right before any operation on the bus in order to reset the internal status.

#### Syntax
    void initLastResult();

#### Parameters
None

#### Returns
None

#### See also
[getLastResult()](#getLastResult)

[setLastResult()](#setLastResult)

[Back to interface](#interface)


<a id="isSuccess"></a>
## isSuccess()
#### Description
The method returns a flag whether the recent operation on the two-wire bus was successful.

#### Syntax
    boolean isSuccess();

#### Parameters
None

#### Returns
Flag about successful processing of the recent operation on the bus.

#### See also
[getLastResult()](#getLastResult)

[isError()](#isError)

[Back to interface](#interface)


<a id="isError"></a>
## isError()
#### Description
The method returns a flag whether the recent operation on the two-wire bus failed. The corresponding error code can be obtained by the method [getLastResult()]((#getLastResult), which is one of error [constants](#constants).

#### Syntax
    boolean isError();

#### Parameters
None

#### Returns
Flag about failing of the recent operation on the bus.

#### See also
[getLastResult()](#getLastResult)

[isSuccess()](#isSuccess)

[Back to interface](#interface)


<a id="setAddress"></a>
## setAddress()
#### Description
The method sets new address of a device in the instance object.

#### Syntax
    uint8_t setAddress(uint8_t address);

#### Parameters
<a id="prm_address"></a>
- **address**: The address value of a device, with which the microcontroller is going to communicate.
  - *Valid values*: Only 7-bit addresses 0 ~ 127 (0x00 ~ 0x7F) are allowed on two-wire bus. Practically the range is from 3 (0x03), in special cases from 1 (0x01) to 119 (0x77), while addresses outside this range are reserved for special purposes.
  - *Default value*: none

#### Returns
Some of [result or error codes](#constants). In fact, it determines whether the new address is correct and a device communicates on that address.

#### Example
```cpp
gbj_twowire Object = gbj_twowire();
if (Object.setAddress(newAddress) == gbj_twowire::SUCCESS)
{
  Serial.println("Success");
}
```

#### See also
[getAddress()](#getAddress)

[getLastResult()](#getLastResult)

[Back to interface](#interface)


<a id="setBusStop"></a>
## setBusStop()
#### Description
The method sets the stop flag determining releasing the two-wire bus after each end of data transmission.

#### Syntax
    void setBusStop(bool busStop);

#### Parameters
<a id="prm_busStop"></a>
- **busStop**: Logical flag about releasing bus after end of transmission.
  - *Valid values*: true, false
    - **true**: Releases the bus after data transmission and enables other master devices to control the bus.
    - **false**: Keeps connection to the bus and enables to begin further data transmission immediately.
  - *Default value*: none

#### Returns
None

#### See also
[constructor](#gbj_twowire)

[getBusStop()](#getBusStop)

[Back to interface](#interface)


<a id="setBusClock"></a>
## setBusClock()
#### Description
The method updates the bus clock frequency in the class instance object only. It takes effect at next bus initialization by using method [setAddress()](#setAddress), [busSend()](#busSend), or [busReceive()](#busReceive).

#### Syntax
    void setBusClock(uint32_t clockSpeed);

#### Parameters
- **clockSpeed**: Two-wire bus clock frequency in Hertz. If the clock is not from enumeration, it fallbacks to 100 kHz.
  - *Valid values*: gbj\_twowire::CLOCK\_100KHZ, gbj\_twowire::CLOCK\_400KHZ
  - *Default value*: none

#### Returns
None

#### See also
[constructor](#gbj_twowire)

[getBusClock()](#getBusClock)

[Back to interface](#interface)


<a id="setPins"></a>
## setPins()
#### Description
The method updates the two-wire bus pins in the class instance object only. They take effect at next bus initialization by using method [busSend()](#busSend) or [busReceive()](#busReceive).

#### Syntax
    void setPins(uint8_t pinSDA, uint8_t pinSCL);

#### Parameters
<a id="prm_pinSDA"></a>
- **pinSDA**: Microcontroller's pin for serial data. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms for communication on the bus. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  - *Valid values*: positive integer
  - *Default value*: none


<a id="prm_pinSCL"></a>
- **pinSCL**: Microcontroller's pin for serial clock. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  - *Valid values*: positive integer
  - *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### See also
[constructor](#gbj_twowire)

[getPinSDA(), getPinSCL()](#getPins)

[Back to interface](#interface)


<a id="setLastResult"></a>
## setLastResult()
#### Description
The method sets the internal status of recent processing on the two-wire bus to input value. Without input parameter it is equivalent to the method [initLastResult()](#initLastResult).

#### Syntax
    uint8_t setLastResult(uint8_t lastResult);

#### Parameters
<a id="prm_lastResult"></a>
- **lastResult**: Desired result code that should be set as a last result code.
  - *Valid values*: Some of [result or error codes](#constants).
  - *Default value*: [gbj\_twowire::SUCCESS](#constants)

#### Returns
New (actual) result code of recent operation on two-wire bus.

#### See also
[initLastResult()](#initLastResult)

[getLastResult()](#getLastResult)

[Back to interface](#interface)


<a id="getAddress"></a>
## getAddress()
#### Description
The method returns the current address of the sensor stored in the class instance object.

#### Syntax
    uint8_t getAddress();

#### Parameters
None

#### Returns
Current address of the sensor.

#### See also
[setAddress()](#setAddress)

[Back to interface](#interface)


<a id="getAddressLimits"></a>
## getAddressMin(), getAddressMinSpecial(), getAddressMinUsual(), getAddressMax(), getAddressMaxUsual()
#### Description
Particular method returns corresponding address limit.

#### Syntax
    uint8_t getAddressMin();  // Minimal technically possible address (0x00)
    uint8_t getAddressMinSpecial();  // Minimal address for specific purpose (0x01)
    uint8_t getAddressMinUsual();  // Minimal address for regular devices (0x03)
    uint8_t getAddressMax();  // Maximal technically possible address (0x7F)
    uint8_t getAddressMaxUsual();  // Maximal address for regular devices (0x77)

#### Parameters
None

#### Returns
Corresponding address limit.

#### See also
[getAddress()](#getAddress)

[Back to interface](#interface)


<a id="getBusStop"></a>
## getBusStop()
#### Description
The method returns the current stopping flag stored in the class instance object.

#### Syntax
    bool getBusStop();

#### Parameters
None

#### Returns
Current stopping flag.

#### See also
[constructor](#gbj_twowire)

[setBusStop()](#setBusStop)

[Back to interface](#interface)


<a id="getBusClock"></a>
## getBusClock()
#### Description
The method returns the current bus clock frequency stored in the class instance object.

#### Syntax
    uint32_t getBusClock();

#### Parameters
None

#### Returns
Current two-wire bus clock frequency in Hertz.

#### See also
[constructor](#gbj_twowire)

[setBusClock()](#setBusStop)

[Back to interface](#interface)


<a id="getPins"></a>
## getPinSDA(), getPinSCL()
#### Description
The particular method returns the current pin (GPIO) number defined for the corresponding two-wire bus wire stored in the class instance object.

#### Syntax
    uint8_t getPinSDA();
    uint8_t getPinSCL();

#### Parameters
None

#### Returns
Current pin (GPIO) number.

#### See also
[constructor](#gbj_twowire)

[setPins()](#setPins)

[Back to interface](#interface)


<a id="getLastResult"></a>
## getLastResult()
#### Description
The method returns a result code of the recent operation on the two-wire bus. It is usually called for error handling in a sketch.

#### Syntax
    uint8_t getLastResult();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

#### Example
```cpp
gbj_twowire Object = gbj_twowire();
Object.initLastResult();
Object.setAddress(newAddress);
if (Object.getLastResult() == gbj_twowire::SUCCESS)
{
  Serial.println("Success");
}
else
{
  Serial.print("Error: ");
  switch (Object.getLastResult())
  {
    case gbj_twowire::ERROR_ADDRESS:
      Serial.println("Bad address");
      break;

    case gbj_twowire::ERROR_NACK_OTHER:
      Serial.println("General error");
      break;

    default:
      Serial.println("Unknown error");
      break;    
  }
}
```

#### See also
[setLastResult()](#setLastResult)

[initLastResult()](#initLastResult)

[Back to interface](#interface)


<a id="getLastCommand"></a>
## getLastCommand()
#### Description
The method returns the command code used at recent communication on two-wire bus. In conjunction with returned result or error code of particular method it is possible to detect the source or reason of a communication error.

#### Syntax
	uint8_t getLastCommand();

#### Parameters
None

#### Returns
Recently used command code.

[Back to interface](#interface)


<a id="busSendStream"></a>
## busSendStream()
#### Description
The method sends input data byte stream to the two-wire bus chunked by parent library two-wire data buffer length (paging).
- If there is a send delay defined, the method waits for that time period expiration before sending next chunk (page).
- In order not to block system, the method does not wait after a transaction, but before transactions for delay expiring. It gives the system a chance to perform some tasks after sending to the bus, which might last the desired delay, so that the method does not block the system uselessly.

#### Syntax
    uint8_t busSendStream(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse);

#### Parameters
- **dataBuffer**: Pointer to the byte data buffer.
  - *Valid values*: address space
  - *Default value*: none


- **dataLen**: Number of bytes to be sent from the data buffer to the bus.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none


- **dataReverse**: Flag about sending the data buffer in reverse order from very last byte (determined by *dataLen*) to the very first byte.
  - *Valid values*: Boolean
    - *false*: sending from the first to the last byte order
    - *true*: sending from the last to the first byte order
  - *Default value*: false

#### Returns
Some of [result or error codes](#constants).

#### See also
[busSendStreamPrefixed()](#busSendStreamPrefixed)

[busSend()](#busSend)

[Back to interface](#interface)


<a id="busSendStreamPrefixed"></a>
## busSendStreamPrefixed()
#### Description
The method sends input data byte array to the two-wire prefixed with prefix buffer.
- The method chunks sent byte stream including prefix by parent library two-wire data buffer length (paging).
- If there is a send delay defined, the method waits for that time period expiration before sending next chunk (page).
- The prefix may be a one-time one, which is used just with the very first chunk only.

#### Syntax
    uint8_t busSendStreamPrefixed(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse, uint8_t *prfxBuffer, uint16_t prfxLen, bool prfxReverse, bool prfxOnetime);

#### Parameters
- **dataBuffer**: Pointer to the byte data buffer.
  - *Valid values*: address space
  - *Default value*: none


- **dataLen**: Number of bytes to be sent from the data buffer to the bus.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none


- **dataReverse**: Flag about sending the data buffer in reverse order from very last byte (determined by *dataLen*) to the very first byte.
  - *Valid values*: Boolean
    - *false*: sending from the first to the last byte order
    - *true*: sending from the last to the first byte order
  - *Default value*: none


- **prfxBuffer**: Pointer to the prefix byte buffer, which precedes sending each or the first data buffer page.
  - *Valid values*: address space
  - *Default value*: none


- **prfxLen**: Number of bytes to be sent from the prefix buffer to the bus. At repeating prefix (non one-time) it is reasonable, if the prefix length is less than two-wire buffer length (usually 32 bytes), otherwise only that prefix is sent to the bus due to paging.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none


- **prfxReverse**: Flag about sending the prefix buffer in reverse order from very last byte (determined by *prfxLen*) to the very first byte.
  - *Valid values*: Boolean
    - *false*: sending from the first to the last byte order
    - *true*: sending from the last to the first byte order
  - *Default value*: none


- **prfxOnetime**: Flag about sending the prefix buffer just once before the very first sending the data buffer.
  - *Valid values*: Boolean
    - *false*: prefix buffer sent before each data buffer page
    - *true*: prefix buffer sent once before start of sending data buffer
  - *Default value*: false

#### Returns
Some of [result or error codes](#constants).

#### See also
[busSendStream()](#busSendStream)

[Back to interface](#interface)


<a id="busSend"></a>
## busSend()
#### Description
The method sends input data to the two-wire bus as one communication transaction.
- The method is overloaded.
- In case of two parameters, the first one is considered as a command and second one as the data. In this case the method sends 2 ~ 4 bytes to the bus in one transaction.
- In case of one parameter, it is considered as the general data and in fact might be a command or the data. In this case the method sends 1 ~ 2 bytes to the bus in one transaction.

#### Syntax
    uint8_t busSend(uint16_t command, uint16_t data);
    uint8_t busSend(uint16_t data);

#### Parameters
- **command**: Word or byte to be sent in the role of command.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none


- **data**: Word or byte to be sent in the role of data.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### See also
[busSendStream()](#busSendStream)

[Back to interface](#interface)


<a id="busRead"></a>
## busRead()
#### Description
The method reads one byte from the two-wire bus.

#### Syntax
    uint8_t busRead();

#### Parameters
None

#### Returns
Data byte read from the bus.

#### See also
[busReceive()](#busReceive)

[Back to interface](#interface)


<a id="busReceive"></a>
## busReceive()
#### Description
The method reads a byte stream from the two-wire bus chunked by parent library two-wire data buffer length (paging) and places them to the buffer defined by an input pointer.

#### Syntax
    uint8_t busReceive(uint8_t *dataBuffer, uint16_t dataLen);

#### Parameters
- **dataBuffer**: Pointer to a byte buffer for storing read data. The buffer should be enough large for storing all read bytes.
  - *Valid values*: address space
  - *Default value*: none


- **dataLen**: Number of bytes to be read.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### Example
```cpp
gbj_twowire Object = gbj_twowire();
uint8_t data[2];
if (Object.busReceive(data, sizeof(data)/sizeof(data[0])))
{
  Serial.println("Data:");
  Serial.println(data[0]);
  Serial.println(data[1]);
}
else
{
  Serial.print("Error: ");
  Serial.println(Object.getLastResult());
}
```

#### See also
[busRead()](#busRead)

[Back to interface](#interface)


<a id="busGeneralReset"></a>
## busGeneralReset()
#### Description
The method sends command 0x06 to the general call address 0x00 in order to execute software reset of all devices on the two-wire bus that have this functionality implemented.

#### Syntax
    uint8_t busGeneralReset();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

[Back to interface](#interface)


<a id="setDelaySend"></a>
## setDelaySend()
#### Description
The method sets delay for waiting before subsequent sending transaction until that time period expires from finishing previous sending transaction.
- In order not to block system, the method does not wait after a transaction, but before transactions for delay expiring. It gives the system a chance to perform some tasks after sending to the bus, which might last the desired delay, so that the method does not block the system uselessly.

#### Syntax
    void setDelaySend(uint32_t delay);

#### Parameters
- **delay**: Delaying time period in milliseconds.
  - *Valid values*: 32 bit unsigned integer.
  - *Default value*: none

#### Returns
None

#### See also
[getDelaySend()](#getDelaySend)

[busSendStream()](#busSendStream)

[Back to interface](#interface)


<a id="getDelaySend"></a>
## getDelaySend()
#### Description
The method returns the current sending delay stored in the class instance object.

#### Syntax
    uint32_t getDelaySend();

#### Parameters
None

#### Returns
Current sending delay in milliseconds.

#### See also
[setDelaySend()](#setDelaySend)

[Back to interface](#interface)


<a id="wait"></a>
## wait()
#### Description
The method waits in the loop until input delay expires.

#### Syntax
    void wait(uint32_t delay);

#### Parameters
- **delay**: Waiting time period in milliseconds.
  - *Valid values*: 32 bit unsigned integer.
  - *Default value*: none

#### Returns
None

[Back to interface](#interface)


<a id="initBus"></a>
## initBus()
#### Description
The method starts two-wire bus, if it is not yet and sets up the flag about it in order not to start the bus again.

#### Syntax
    void initBus();

#### Parameters
None

#### Returns
None

[Back to interface](#interface)
