# gbjTwoWire
The library embraces and provides common methods used at every application working with sensor on `two-wire` (also known as <abbr title='Inter-Integrated Circuit'>I2C</abbr>) bus.

* Library specifies (inherits from) the system `TwoWire` library from the file `Wire.h`.
* The class from the library is not intended to be used directly in a sketch, just as a parent class for specific sensor libraries.
* Library respects two-wire buffer length (`32 bytes`) at communication on the bus by paging, so that it splits long byte streams into separate transmissions.
* Library implements extended error handling.
* Library provides some general system methods implemented differently for various platforms, especially for ones with hardware two-wire bus implementation (Arduino, Particle - Photon, Electron...) and for ones with software (bit-banged) defined two-wire bus (Espressif - ESP8266, ESP32).
* Library initiates the two-wire bus at the default speed (serial clock) `100 kHz` and with generating stop condition after every end of transmission or data request, but they can be changed dynamically.
* It is expected, that initialization of a device and setting its parameters is provided in a child bclass inherited from this class defined in that library in the method `begin`.
* Library does not use the built-in function `delay()` at waiting for some actions, e.g., waking up sensor from power down mode, but instead of it uses the own implementation of the `wait()` function based on system `millis()` function.
* Library allows address range `0x00 ~ 0x7F`.
* Library utilizes the general call on the two-wire bus as well.


<a id="dependency"></a>
## Dependency

#### Particle platform
* **Particle.h**: Includes alternative (C++) data type definitions.

#### Arduino platform
* **Arduino.h**: Main include file for the Arduino SDK version greater or equal to 100.
* **inttypes.h**: Integer type conversions. This header file includes the exact-width integer definitions and extends them with additional facilities provided by the implementation.
* **TwoWire**: I2C system library loaded from the file `Wire.h`.

#### Espressif platform
* **Arduino.h**: Main include file for the Arduino SDK version greater or equal to 100.
* **TwoWire**: I2C system library loaded from the file `Wire.h`.


<a id="constants"></a>

## Constants
* **ClockSpeeds::CLOCK\_100KHZ**: Bus clock speed 100 kHz.
* **ClockSpeeds::CLOCK\_400KHZ**: Bus clock speed 400 kHz.
* **ResultCodes::SUCCESS**: Result code for successful processing.

### Arduino and Espressif errors
* **ResultCodes::ERROR\_BUFFER**: Data too long to fit in transmit buffer.
* **ResultCodes::ERROR\_NACK\_ADDR**: Received NACK on transmit of address.
* **ResultCodes::ERROR\_NACK\_DATA**: Received NACK on transmit of data.
* **ResultCodes::ERROR\_NACK\_OTHER**:  Other error.

### Particle errors
* **ResultCodes::ERROR\_BUSY**: Busy timeout upon entering `endTransmission()`.
* **ResultCodes::ERROR\_START**: START bit generation timeout.
* **ResultCodes::ERROR\_END**: End of address transmission timeout.
* **ResultCodes::ERROR\_TRANSFER**: Data byte transfer timeout.
* **ResultCodes::ERROR\_TIMEOUT**: Data byte transfer succeeded, busy timeout immediately after.

### Generic errors
* **ResultCodes::ERROR\_ADDRESS**: Platform specific error code at bad address.
* **ResultCodes::ERROR\_PINS**: For software defined two-wire bus platforms at bad defined bus pins (<abbr title='General Purpose Input Output'>GPIO</abbr>), usually both are the same.
* **ResultCodes::ERROR\_RCV\_DATA**: Received data is zero or shorter than expected.
* **ResultCodes::ERROR\_POSITION**: Wrong position in memory; either 0 or no sufficient space for data storing or retrieving.
* **ResultCodes::ERROR\_DEVICE**: Wrong device type or other device fault.
* **ResultCodes::ERROR\_RESET**: Device reset failure.
* **ResultCodes::ERROR\_FIRMWARE**: Device's firmware reading failure.
* **ResultCodes::ERROR\_SN**: Device's serial number reading failure.
* **ResultCodes::ERROR\_MEASURE**: Measuring by a device failure.
* **ResultCodes::ERROR\_REGISTER**: Device's register operation failure.

The library class comprises in generic error codes all potential error codes from derived classes, i.e., hardware sensors' libraries.


### Referencing constants
In a sketch the constants can be referenced in following forms:
* **Static constant** in the form `gbj_twowire::<enumeration>::<constant>` or shortly `gbj_twowire::<constant>`, e.g., _gbj_twowire::ResultCodes::SUCCESS_ or _gbj_twowire::SUCCESS_.
* **Instance constant** in the form `<object>.<constant>`, e.g., _object.SUCCESS_.
```cpp
gbj_twowire object = gbj_twowire(object.CLOCK_400KHZ);
setup()
{
  if (object.begin() == object.SUCCESS)
  {
    ...
  }
}
```

<a id="interface"></a>

## Interface
* [gbj_twowire()](#gbj_twowire)
* [~gbj_twowire()](#gbj_twowire)
* [begin()](#begin)
* [release()](#release)
* [busSendStream()](#busSendStream)
* [busSendStreamPrefixed()](#busSendStreamPrefixed)
* [busSend()](#busSend)
* [busReceive()](#busReceive)
* [busGeneralReset()](#busGeneralReset)
* [registerAddress()](#registerAddress)

#### Setters
* [setLastResult()](#setLastResult)
* [setAddress()](#setAddress)
* [setBusClock()](#setBusClock)
* [setPins()](#setPins)
* [setDelaySend()](#setDelay)
* [setDelayReceive()](#setDelay)

#### Getters
* [getLastResult()](#getLastResult)
* [getLastErrorTxt()](#getLastErrorTxt)
* [getLastCommand()](#getLastResult)
* [getAddress()](#getAddress)
* [getAddressMin()](#getAddressLimits)
* [getAddressMinSpecial()](#getAddressLimits)
* [getAddressMinUsual()](#getAddressLimits)
* [getAddressMax()](#getAddressLimits)
* [getAddressMaxUsual()](#getAddressLimits)
* [getBusClock()](#getBusClock)
* [getPinSDA()](#getPins)
* [getPinSCL()](#getPins)
* [isSuccess()](#isSuccess)
* [isError()](#isError)

#### Protected
* [setBusStop()](#setBusStop)
* [setBusRepeat()](#setBusStop)
* [setBusStopFlag()](#setBusStop)
* [getBusStop()](#getBusStop)
* [getDelaySend()](#getDelay)
* [getDelayReceive()](#getDelay)
* [setTimestamp()](#setTimestamp)
* [getTimestamp()](#getTimestamp)
* [waitTimestampSend()](#waitTimestamp)
* [waitTimestampReceive()](#waitTimestamp)
* [wait()](#wait)
* [initBus()](#initBus)


<a id="gbj_twowire"></a>

## gbj_twowire(), ~gbj_twowire()

#### Description
Destructor `~gbj_twowire()` just releases the two-wire bus.
Constructor `gbj_twowire()` creates the class instance object and sets some bus parameters that are usually constant in a sketch. On the other hand, constructor parameters can be changed individually later in a sketch, if it is needed to change them dynamically.
* If subclass inherited from this class does not need special constructor or destructor, that class does not need to define constructor and destructor whatsoever.

#### Syntax
    gbj_twowire(ClockSpeeds clockSpeed, uint8_t pinSDA, uint8_t pinSCL)

#### Parameters
* **clockSpeed**: Initial two-wire bus clock frequency in Hertz.
  * *Valid values*: ClockSpeeds::CLOCK\_100KHZ, ClockSpeeds::CLOCK\_400KHZ
  * *Default value*: ClockSpeeds::CLOCK\_100KHZ

* **pinSDA**: Microcontroller's pin for serial data. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms for communication on the bus. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  * *Valid values*: positive integer
  * *Default value*: 4 (GPIO4, D2)

* **pinSCL**: Microcontroller's pin for serial clock. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  * *Valid values*: positive integer
  * *Default value*: 5 (GPIO5, D1)

#### Returns
Object performing the extended two-wire bus management.
The constructor cannot return [a result or error code](#constants) directly, however, it stores them in the instance object. The result can be tested in the operational code with the method [getLastResult()](#getLastResult), [isError()](#isError), or [isSuccess()](#isSuccess).

[Back to interface](#interface)


<a id="begin"></a>

## begin()

#### Description
The method checks the microcontroller's pins defined in the [constructor](gbj_twowire) for software defined two-wire bus platforms and initiates the two-wire bus. So that, it should be called at the beginning of every corresponding method of an inherited class.
* The method checks whether pins set by constructor are not mutually equal.
* For hardware two-wire bus platforms the method ignores defined or default pins.

#### Syntax
	ResultCodes begin()

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

#### Example
Calling methods in an application subclass.
```cpp
uint8_t gbj_acme::begin()
{
  if (gbj_twowire::begin()) return getLastResult();
  ...
}
```

[Back to interface](#interface)


<a id="release"></a>

## release()

#### Description
The method releases the two-wire bus, so that pins used by it are available for general purpose I/O.

#### Syntax
    void release()

#### Parameters
None

#### Returns
None

[Back to interface](#interface)


<a id="isSuccess"></a>

## isSuccess()

#### Description
The method returns a flag whether the recent operation on the two-wire bus was successful.
- The method is overloaded. If the input argument is used, it is used and internally saved instead of recently saved code.
- The method with input argument is useful for calling inline method or function returning the code.

#### Syntax
    boolean isSuccess()
    boolean isSuccess(ResultCodes lastResult)

#### Parameters
* **lastResult**: Result code of a recent operation usually returned by a method or function.
  * *Valid values*: Some of [result or error codes](#constants).
  * *Default value*: none

#### Returns
Flag about successful processing of the recent operation on the bus.

#### See also
[getLastResult()](#getLastResult)

[isError()](#isError)

[Back to interface](#interface)


<a id="isError"></a>

## isError()

#### Description
The method returns a flag whether the recent operation on the two-wire bus failed. The corresponding error code can be obtained by the method [getLastResult()](#getLastResult), which is one of error [constants](#constants).
- The method is overloaded. If the input argument is used, it is used and internally saved instead of recently saved code.
- The method with input argument is useful for calling inline method or function returning the code.

#### Syntax
    boolean isError()
    boolean isError(ResultCodes lastResult)

#### Parameters
* **lastResult**: Result code of a recent operation usually returned by a method or function.
  * *Valid values*: Some of [result or error codes](#constants).
  * *Default value*: none

#### Returns
Flag about failing of the recent operation on the bus.

#### See also
[getLastResult()](#getLastResult)

[isSuccess()](#isSuccess)

[Back to interface](#interface)


<a id="registerAddress"></a>

## registerAddress()

#### Description
The method stores a new address of a device in the instance object without any activity on the two-wire bus. It just checks whether the address is within valid range.

#### Syntax
    ResultCodes registerAddress(uint8_t address)

#### Parameters
* **address**: The address value of a device, with which the microcontroller is going to communicate.
  * *Valid values*: Only 7-bit addresses `0 ~ 127` (0x00 ~ 0x7F) are allowed on two-wire bus. Practically the range is from 3 (0x03), in special cases from 1 (0x01) to 119 (0x77), while addresses outside this range are reserved for special purposes.
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### Example
```cpp
gbj_twowire object = gbj_twowire();
object.registerAddress(newAddress);
if (object.isSuccess())
{
  Serial.println("Success");
}
```

#### See also
[setAddress()](#setAddress)

[Back to interface](#interface)


<a id="setAddress"></a>

## setAddress()

#### Description
The method checks right after [registering](#registerAddress()) a new address if a device communicates with it on the two-wire bus.

#### Syntax
    ResultCodes setAddress(uint8_t address)

#### Parameters
* **address**: See the same parameter of the method [registerAddress](#registerAddress).

#### Returns
Some of [result or error codes](#constants).

#### See also
[registerAddress()](#registerAddress)

[getAddress()](#getAddress)

[Back to interface](#interface)


<a id="setBusStop"></a>

## setBusStop(), setBusRepeat(), setBusStopFlag()

#### Description
The particular method sets the corresponding flag whether stop or repeated start condition should be generated after each end of data transmission or data request on the two-wire bus. The last one is useful at returning back the original flag in procedures that temporary change it.

#### Syntax
    void setBusStop()
    void setBusRepeat()
    void setBusStopFlag(bool busStop)

#### Returns
* **busStop**: Flag about stop or repeating start condition.
  * *Valid values*: Boolean
    * *true*: generating stop condition at the end of transmission on the two-wire bus
    * *false*: generating repeated start condition at the end of transmission on the two-wire bus
  * *Default value*: none

#### See also
[getBusStop()](#getBusStop)

[Back to interface](#interface)


<a id="setBusClock"></a>

## setBusClock()

#### Description
The method updates the bus clock frequency in the class instance object only. It takes effect at next bus initialization by using method [setAddress()](#setAddress), [busSend()](#busSend), or [busReceive()](#busReceive).

#### Syntax
    void setBusClock(ClockSpeeds clockSpeed)

#### Parameters
* **clockSpeed**: Two-wire bus clock frequency in Hertz. If the clock is not from enumeration, it fallbacks to 100 kHz.
  * *Valid values*: ClockSpeeds::CLOCK\_100KHZ, ClockSpeeds::CLOCK\_400KHZ
  * *Default value*: none

#### Returns
None

#### See also
[gbj_twowire](#gbj_twowire)

[getBusClock()](#getBusClock)

[Back to interface](#interface)


<a id="setPins"></a>

## setPins()

#### Description
The method updates the two-wire bus pins in the class instance object only. They take effect at next bus initialization by using method [busSend()](#busSend) or [busReceive()](#busReceive).

#### Syntax
    void setPins(uint8_t pinSDA, uint8_t pinSCL)

#### Parameters
* **pinSDA**: Microcontroller's pin for serial data. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms for communication on the bus. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  * *Valid values*: positive integer
  * *Default value*: none

* **pinSCL**: Microcontroller's pin for serial clock. It is not a board pin but GPIO number. For hardware two-wire bus platforms it is irrelevant and none of methods utilizes this parameter for such as platforms. On the other hand, for those platforms the parameters might be utilized for storing some specific attribute in the class instance object.
  * *Valid values*: positive integer
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### See also
[gbj_twowire](#gbj_twowire)

[getPinSDA(), getPinSCL()](#getPins)

[Back to interface](#interface)


<a id="setLastResult"></a>

## setLastResult()

#### Description
The method sets and returns the internal status of recent processing on the two-wire bus to input value.
* Without input parameter the method sets the [ResultCodes::SUCCESS](#constants) code. It is usually called right before any operation on the bus in order to reset the internal status or in methods without bus communication.

#### Syntax
    ResultCodes setLastResult(ResultCodes lastResult)

#### Parameters
* **lastResult**: Desired result code that should be set as a last result code.
  * *Valid values*: Some of [result or error codes](#constants).
  * *Default value*: [ResultCodes::SUCCESS](#constants)

#### Returns
New (actual) result code of recent operation on two-wire bus.

#### See also
[getLastResult()](#getLastResult)

[Back to interface](#interface)


<a id="getAddress"></a>

## getAddress()

#### Description
The method returns the current address of the sensor stored in the class instance object.

#### Syntax
    uint8_t getAddress()

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
The method returns a flag about currently generating stop or repeated start condition.

#### Syntax
    bool getBusStop()

#### Parameters
None

#### Returns
Current stopping or repeated start flag.

#### See also
[setBusStop(), setBusRepeat()](#setBusStop)

[Back to interface](#interface)


<a id="getBusClock"></a>

## getBusClock()

#### Description
The method returns the current bus clock frequency stored in the class instance object in Hertz.

#### Syntax
    ClockSpeeds getBusClock()

#### Parameters
None

#### Returns
Current two-wire bus clock frequency in Hertz.

#### See also
[gbj_twowire](#gbj_twowire)

[setBusClock()](#setBusStop)

[Back to interface](#interface)


<a id="getPins"></a>

## getPinSDA(), getPinSCL()

#### Description
The particular method returns the current pin (GPIO) number defined for the corresponding two-wire bus wire stored in the class instance object.

#### Syntax
    uint8_t getPinSDA()
    uint8_t getPinSCL()

#### Parameters
None

#### Returns
Current pin (GPIO) number.

#### See also
[gbj_twowire](#gbj_twowire)

[setPins()](#setPins)

[Back to interface](#interface)


<a id="getLastResult"></a>

## getLastResult()

#### Description
The method returns a result code of the recent operation on the two-wire bus. It is usually called for error handling in a sketch.

#### Syntax
    ResultCodes getLastResult()

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

#### Example
```cpp
gbj_twowire object = gbj_twowire();
uint8_t newAddress = 0x24;
object.setLastResult();
object.setAddress(newAddress);
if (object.getLastResult() == gbj_twowire::ResultCodes::SUCCESS)
{
  Serial.println("Success");
}
else
{
  Serial.print("Error: ");
  switch (object.getLastResult())
  {
    case gbj_twowire::ResultCodes::ERROR_ADDRESS:
      Serial.println("Bad address");
      break;

    case gbj_twowire::ResultCodes::ERROR_NACK_OTHER:
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

[Back to interface](#interface)


<a id="getLastErrorTxt"></a>

## getLastErrorTxt()

#### Description
The method translates internally stored error code of the recent operation to corresponding wording.

#### Syntax
    String getLastErrorTxt(String location)

#### Parameters
* **location**: Location of the error code in a sketch utilized as an error text prefix.
  * *Valid values*: String
  * *Default value*: empty string

#### Returns
Textual representation or wording of the error code.

[Back to interface](#interface)


<a id="getLastCommand"></a>

## getLastCommand()

#### Description
The method returns the command code used at recent communication on two-wire bus. In conjunction with returned result or error code of particular method it is possible to detect the source or reason of a communication error.

#### Syntax
	uint16_t getLastCommand()

#### Parameters
None

#### Returns
Recently used command code.

[Back to interface](#interface)


<a id="busSendStream"></a>

## busSendStream()

#### Description
The method sends input data byte stream to the two-wire bus chunked by parent library two-wire data buffer length (paging).
* If there is a send delay defined, the method waits for that time period expiration before sending next chunk (page).
* In order not to block system, the method does not wait after a transmission, but before transmissions for delay expiring. It gives the system a chance to perform some tasks after sending to the bus, which might last the desired delay, so that the method does not block the system uselessly.

#### Syntax
    ResultCodes busSendStream(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse)

#### Parameters
* **dataBuffer**: Pointer to the byte data buffer.
  * *Valid values*: address space
  * *Default value*: none

* **dataLen**: Number of bytes to be sent from the data buffer to the bus.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

* **dataReverse**: Flag about sending the data buffer in reverse order from very last byte (determined by *dataLen*) to the very first byte.
  * *Valid values*: Boolean
    * *false*: sending from the first to the last byte order
    * *true*: sending from the last to the first byte order
  * *Default value*: false

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
* The method chunks sent byte stream including prefix by parent library two-wire data buffer length (paging).
* If there is a send delay defined, the method waits for that time period expiration before sending next chunk (page).
* The prefix may be a one-time one, which is used just with the very first chunk only.

#### Syntax
    ResultCodes busSendStreamPrefixed(uint8_t *dataBuffer, uint16_t dataLen, bool dataReverse, uint8_t *prfxBuffer, uint16_t prfxLen, bool prfxReverse, bool prfxOnetime)

#### Parameters
* **dataBuffer**: Pointer to the byte data buffer.
  * *Valid values*: address space
  * *Default value*: none

* **dataLen**: Number of bytes to be sent from the data buffer to the bus.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

* **dataReverse**: Flag about sending the data buffer in reverse order from very last byte (determined by *dataLen*) to the very first byte.
  * *Valid values*: Boolean
    * *false*: sending from the first to the last byte order
    * *true*: sending from the last to the first byte order
  * *Default value*: none

* **prfxBuffer**: Pointer to the prefix byte buffer, which precedes sending each or the first data buffer page.
  * *Valid values*: address space
  * *Default value*: none

* **prfxLen**: Number of bytes to be sent from the prefix buffer to the bus. At repeating prefix (non one-time) it is reasonable, if the prefix length is less than two-wire buffer length (usually 32 bytes), otherwise only that prefix is sent to the bus due to paging.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

* **prfxReverse**: Flag about sending the prefix buffer in reverse order from very last byte (determined by *prfxLen*) to the very first byte.
  * *Valid values*: Boolean
    * *false*: sending from the first to the last byte order
    * *true*: sending from the last to the first byte order
  * *Default value*: none

* **prfxOnetime**: Flag about sending the prefix buffer just once before the very first sending the data buffer.
  * *Valid values*: Boolean
    * *false*: prefix buffer sent before each data buffer page
    * *true*: prefix buffer sent once before start of sending data buffer
  * *Default value*: false

#### Returns
Some of [result or error codes](#constants).

#### See also
[busSendStream()](#busSendStream)

[Back to interface](#interface)


<a id="busSend"></a>

## busSend()

#### Description
The method sends input data to the two-wire bus as one communication transmission.
* The method is overloaded.
* In case of two parameters, the first one is considered as a command and second one as the data. In this case the method sends 2 ~ 4 bytes to the bus in one transmission.
* In case of one parameter, it is considered as a command, but it can be the general data. In this case the method sends 1 ~ 2 bytes to the bus in one transmission.

#### Syntax
    ResultCodes busSend(uint16_t command, uint16_t data)
    ResultCodes busSend(uint16_t command)

#### Parameters
* **command**: Word or byte to be sent in the role of command.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

* **data**: Word or byte to be sent in the role of data.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### See also
[busSendStream()](#busSendStream)

[Back to interface](#interface)


<a id="busReceive"></a>

## busReceive()

#### Description
The method reads a byte stream from the two-wire bus chunked by parent library two-wire data buffer length (paging) and places them to the buffer defined by an input pointer.
* The method is overloaded.
* In case of 3 parameters, the first one is considered as a command, which is sent to the bus before reading from it.
* In case of 2 parameter, the method just reads to the buffer from the bus.

#### Syntax
    ResultCodes busReceive(uint16_t command, uint8_t *dataBuffer, uint16_t dataLen)
    ResultCodes busReceive(uint8_t *dataBuffer, uint16_t dataLen)

#### Parameters
* **command**: Word or byte to be sent to the two-wire bus in the role of command.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

* **dataBuffer**: Pointer to a byte buffer for storing read data. The buffer should be enough large for storing all read bytes.
  * *Valid values*: address space
  * *Default value*: none

* **dataLen**: Number of bytes to be read.
  * *Valid values*: non-negative integer 0 ~ 65535
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants).

#### Example
```cpp
gbj_twowire object = gbj_twowire();
uint8_t data[2];
if (object.busReceive(data, sizeof(data)/sizeof(data[0])))
{
  Serial.println("Data:");
  Serial.println(data[0]);
  Serial.println(data[1]);
}
else
{
  Serial.print("Error: ");
  Serial.println(object.getLastResult());
}
```

#### See also
[busSend()](#busSend)

[Back to interface](#interface)


<a id="busGeneralReset"></a>

## busGeneralReset()

#### Description
The method sends command `0x06` to the general call address `0x00` in order to execute software reset of all devices on the two-wire bus that have this functionality implemented.

#### Syntax
    ResultCodes busGeneralReset()

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

[Back to interface](#interface)


<a id="setDelay"></a>

## setDelaySend(), setDelayReceive()

#### Description
The particular method sets delay for waiting before subsequent sending or receving transmission until that time period expires from finishing that previous transmission.
* In order not to block system, the method does not wait after a transmission, but before transmissions for delay expiring. It gives the system a chance to perform some tasks after communication on the bus, which might last the desired delay, so that the method does not block the system uselessly.
* In order to reset the delay, put 0 to input argument.

#### Syntax
    void setDelaySend(uint32_t delay)
    void setDelayReceive(uint32_t delay)

#### Parameters
* **delay**: Delaying time period in milliseconds.
  * *Valid values*: 32 bit unsigned integer
  * *Default value*: none

#### Returns
None

#### See also
[getDelaySend(), getDelayReceive()](#getDelay)

[busSendStream()](#busSendStream)

[busReceive()](#busReceive)

[Back to interface](#interface)


<a id="getDelay"></a>

## getDelaySend(), getDelayReceive()

#### Description
The particular method returns the current sending or receiving delay stored in the class instance object.

#### Syntax
    uint32_t getDelaySend()
    uint32_t getDelayReceive()

#### Parameters
None

#### Returns
Particular current delay in milliseconds.

#### See also
[setDelaySend(), setDelayReceive()](#setDelay)

[Back to interface](#interface)


<a id="setTimestamp"></a>

## setTimestamp()

#### Description
The method sets or erases internal timestamp of just finished communication transmission on the two-wire bus to the current running time of the microcontroller in milliseconds with the function `millis()`.

#### Syntax
    void setTimestamp()

#### Parameters
None

#### Returns
None

#### See also
[waitTimestampSend(), waitTimestampReceive()](#waitTimestamp)

[Back to interface](#interface)


<a id="waitTimestamp"></a>

## waitTimestampSend(), waitTimestampReceive()

#### Description
The particular method waits until current running time of the microcontroller reaches internal sending or receiving timestamp. * The method enables sensors to settle after changing their status.

#### Syntax
    void waitTimestampSend()
    void waitTimestampReceive()

#### Parameters
None

#### Returns
None

#### See also
[setTimestamp()](#setTimestamp)

[wait()](#wait)

[Back to interface](#interface)


<a id="getTimestamp"></a>

## getTimestamp()

#### Description
The method returns recently internally saved timestamp of a recent transmission.

#### Syntax
    uint32_t getTimestamp()

#### Parameters
None

#### Returns
Timestamp of the recent finishedcommunication transmission on the two-wire bus.

#### See also
[setTimestamp()](#setTimestamp)

[wait()](#wait)

[Back to interface](#interface)


<a id="wait"></a>

## wait()

#### Description
The method waits in the loop until input delay expires.

#### Syntax
    void wait(uint32_t delay)

#### Parameters
* **delay**: Waiting time period in milliseconds.
  * *Valid values*: 32 bit unsigned integer.
  * *Default value*: none

#### Returns
None

#### See also
[waitTimestampSend(), waitTimestampReceive()](#waitTimestamp)

[Back to interface](#interface)


<a id="initBus"></a>

## initBus()

#### Description
The method starts two-wire bus, if it is not yet started and sets up the flag about it in order not to start the bus again.

#### Syntax
    void initBus()

#### Parameters
None

#### Returns
None

[Back to interface](#interface)
