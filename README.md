<a id="library"></a>
# gbjTwoWire
Library embraces and provides common methods used at every application working with sensor on two wire (I2C) bus.
- Library specifies (inherits from) the system TwoWire library.
- The class from the library is not intended to be used directly in a sketch, just as a parent class for specific sensor libraries.
- Library implements extended error handling.
- Library provides some general system methods implemented differently for various platforms, especially Arduino vs. Particle.
- The library does not use the built-in function `delay()` at waiting for waking up sensor from power down mode, but instead of it the own implementation of the `wait()` function based on system `millis()` function.
- The library initiates the two wire bus at default speed (serial clock) **100 kHz**.
- It is expected, that initialization of a device and setting its parameters is provided in a subclass inherited from the class defined in this library in the method `begin`, which is not defined in the library.



<a id="dependency"></a>
## Dependency

#### Particle platform
- **Particle.h**: Includes alternative (C++) data type definitions.

#### Arduino platform
- **Arduino.h**: Main include file for the Arduino SDK version greater or equal to 100.
- **WProgram.h**: Main include file for the Arduino SDK version less than 100.
- **inttypes.h**: Integer type conversions. This header file includes the exact-width integer definitions and extends them with additional facilities provided by the implementation.
- **TwoWire**: I2C system library loaded from the file *Wire.h*.


<a id="constants"></a>
## Constants
- **GBJ\_TWOWIRE\_VERSION**: Name and semantic version of the library.
- **GBJ\_TWOWIRE\_ADDRESS\_MIN**: Minimal valid address of a device.
- **GBJ\_TWOWIRE\_ADDRESS\_MAX**: Maximal valid address of a device.
- **CLOCK\_SPEED\_100KHZ**: Bus clock speed 100 kHz.
- **CLOCK\_SPEED\_400KHZ**: Bus clock speed 400 kHz.
- **GBJ\_TWOWIRE\_SUCCESS**: Result code for successful processing.
- **GBJ\_TWOWIRE\_ERR\_ADDRESS**: Platform specific error code at bad address

### Arduino error codes
- **GBJ\_TWOWIRE\_ERR\_BUFFER**: Data too long to fit in transmit buffer.
- **GBJ\_TWOWIRE\_ERR\_NACK\_ADDR**: Received NACK on transmit of address.
- **GBJ\_TWOWIRE\_ERR\_NACK\_DATA**: Received NACK on transmit of data.
- **GBJ\_TWOWIRE\_ERR\_OTHER**:  Other error.

### Particle error codes
- **GBJ\_TWOWIRE\_ERR\_BUSY**: Busy timeout upon entering endTransmission().
- **GBJ\_TWOWIRE\_ERR\_START**: START bit generation timeout.
- **GBJ\_TWOWIRE\_ERR\_END**: End of address transmission timeout.
- **GBJ\_TWOWIRE\_ERR\_TRANSFER**: Data byte transfer timeout.
- **GBJ\_TWOWIRE\_ERR\_TIMEOUT**: Data byte transfer succeeded, busy timeout immediately after.

Remaining constants are listed in the library include file. They are used mostly internally as function codes of the sensor.


<a id="interface"></a>
## Interface
- [gbj_twowire()](#gbj_twowire)
- [~gbj_twowire()](#gbj_twowire)
- [release()](#release)
- [initLastResult()](#initLastResult)
- [isSuccess()](#isSuccess)
- [isError()](#isError)
- [busWrite()](#busWrite)
- [busSend()](#busSend)
- [busRead()](#busRead)
- [busReceive()](#busReceive)

#### Setters
- [setAddress()](#setAddress)
- [setBusStop()](#setBusStop)
- [setLastResult()](#setLastResult)

#### Getters
- [getAddress()](#getAddress)
- [getBusStop()](#getBusStop)
- [getLastResult()](#getLastResult)


<a id="gbj_twowire"></a>
## gbj_twowire()
#### Description
Constructor `gbj_twowire()` just creates a class instance object.
Destructor `~gbj_twowire()` just releases the two wire bus.
- If subclass inherited from this class does not need special constructor or destructor, that class does not need to define constructor and destructor whatsoever.

#### Syntax
    gbj_twowire();

#### Parameters
None

#### Returns
Object performing the extended two wire bus management.

[Back to interface](#interface)


<a id="release"></a>
## release()
#### Description
The method releases the two wire bus so that pins used by it are available for general purpose I/O.

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
The method sets internal status of recent processing on the two wire bus to success with value of macro [GBJ\_TWOWIRE\_SUCCESS](#constants). It is usually called right before any operation on the bus in order to reset the internal status.

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
The method returns a flag whether the recent operation on the two wire bus was successful.

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
The method returns a flag whether the recent operation on the two wire bus failed. The corresponding error code can be obtained by the method [getLastResult()]((#getLastResult), which one of the error macro [constants](#constants).

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
The method sets new address of a device to the instance object.

#### Syntax
    uint8_t setAddress(uint8_t address);

#### Parameters
<a id="prm_address"></a>
- **address**: The address value of a device, with which the microcontroller is going to communicate.
  - *Valid values*: [GBJ\_TWOWIRE\_ADDRESS\_MIN ~ GBJ\_TWOWIRE\_ADDRESS\_MAX](#constants). Only 7-bit addresses 0 ~ 127 (0x00 ~ 0x7F) are allowed on two wire bus. Practically the range is from 3 (0x03), in special cases from 1 (0x01) to 119 (0x77), while addresses outside this range are reserved for special purposes.
  - *Default value*: none

#### Returns
Result code defined by some of the macro [constants](#constants). In fact, it determines whether the new address is correct and a device communicate on that address.

#### Example
```cpp
gbj_twowire Object = gbj_twowire();
if (Object.setAddress(newAddress) == GBJ_TWOWIRE_SUCCESS)
{
  Serial.println("Success");
}
```

#### See also
[getAddress()](#getAddress)

[Back to interface](#interface)


<a id="setBusStop"></a>
## setBusStop()
#### Description
The method sets the stop flag determining releasing the two wire bus after each end of data transmission.

#### Syntax
    bool setBusStop(bool busStop);

#### Parameters
<a id="prm_busStop"></a>
- **busStop**: Logical flag about releasing bus after end of transmission.
  - *Valid values*: true, false
	  - **true**: Releases the bus after data transmission and enables other master devices to control the bus.
    - **false**: Keeps connection to the bus and enables begin further data transmission immediately.
  - *Default value*: none

#### Returns
New (actual) value of bus stop flag.

#### See also
[getBusStop()](#getBusStop)

[Back to interface](#interface)


<a id="setLastResult"></a>
## setLastResult()
#### Description
The method sets the internal status of recent processing on the two wire bus to input value. Without input parameter it is equivalent to the method [initLastResult()](#initLastResult).

#### Syntax
    uint8_t setLastResult(uint8_t lastResult);

#### Parameters
<a id="prm_lastResult"></a>
- **lastResult**: Desired result code that should be set as a last result code.
  - *Valid values*: One of macro for [result codes](#constants).
  - *Default value*: [GBJ\_TWOWIRE\_SUCCESS](#constants)

#### Returns
New (actual) result code of recent operation on two wire bus.

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
[setBusStop()](#setBusStop)

[Back to interface](#interface)


<a id="getLastResult"></a>
## getLastResult()
#### Description
The method returns a result code of the recent operation on the two wire bus. It is usually called for error handling in a sketch.

#### Syntax
    uint8_t getLastResult();

#### Parameters
None

#### Returns
Current result code. It is one of expected [result codes](#constants).

#### Example
```cpp
gbj_twowire Object = gbj_twowire();
Object.initLastResult();
Object.setAddress(newAddress);
if (Object.getLastResult() == GBJ_TWOWIRE_SUCCESS)
{
  Serial.println("Success");
}
else
{
  Serial.print("Error: ");
  switch (Object.getLastResult())
  {
    case GBJ_TWOWIRE_ERR_NACK_ADDR:
      Serial.println("Bad address");
      break;

    case GBJ_TWOWIRE_ERR_OTHER:
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


<a id="busWrite"></a>
## busWrite()
#### Description
The method writes one or two byte integer data to the two-wire bus in respect to the current platform.
- If the most significant byte (the first one from the left) is non-zero, the data is written as two subsequent bytes.
- If the most significant byte is zero, the data is written as its Least significant byte (the right most one).

#### Syntax
    uint8_t busWrite(uint16_t data);

#### Parameters
<a id="prm_data"></a>
- **data**: Data word or byte to be written.
  - *Valid values*: non-negative integer 0 ~ 65535
  - *Default value*: none

#### Returns
Number of transmitted bytes.

#### See also
[busSend()](#busSend)

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
Current result code. It is one of expected [result codes](#constants).

#### See also
[busWrite()](#busWrite)

[Back to interface](#interface)


<a id="busRead"></a>
## busRead()
#### Description
The method reads one byte from the two-wire bus in respect to the current platform.

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
The method reads multiple bytes from the two-wire bus and places them to the array defined by an input pointer.
- The read bytes are put into the input array from starting index or from the beginning.

#### Syntax
    uint8_t busReceive(uint8_t dataArray[], uint8_t bytes, uint8_t start);

#### Parameters
- **dataArray**: Pointer to an array of bytes for storing read data. The array should be enough large for storing all read bytes.
  - *Valid values*: address of array of non-negative integers each 0 ~ 255 specific for the current platform
  - *Default value*: none

- **bytes**: Number of bytes to be read.
  - *Valid values*: non-negative integer 0 ~ 255
  - *Default value*: none

- **start**: The array index where to start storing read bytes.
  - *Valid values*: non-negative integer 0 ~ 255
  - *Default value*: 0

#### Returns
Current result code. It is one of expected [result codes](#constants).

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

```cpp
gbj_twowire Object = gbj_twowire();
uint8_t data[5];
if (Object.busReceive(data, 2, 3))
{
  Serial.println("Data:");
  Serial.println(data[3]);
  Serial.println(data[4]);
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
