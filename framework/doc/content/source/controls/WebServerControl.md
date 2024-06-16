# WebServerControl

The `WebServerControl` object is designed to allow an external process to control a MOOSE solve. It works by starting up a webserver which exposes a REST API. As with all REST APIs the input and output are both JSON.

The server can either listen on a port via the [!param](/Controls/WebServerControl/port) parameter, or on a unix file socket via the [!param](/Controls/WebServerControl/file_socket) parameter. One of these two parameters must be provided.

It can then be managed via the [MooseControl](MooseControl/index.md optional=true) python utility.

## API

The `WebServerControl` presents REST endpoints to help control the solve.

For the endpoints that follow, if the HTTP status code received is a non-successful code, there may be a response of type `application/json` of the form:

```language=json
{
  "error": "<ERROR>"
}
```

where `<ERROR>` is a useful error message pertaining to the error that was encountered. The response is not guaranteed to contain `application/json` data, but it will when more context is available.

The REST endpoints are as follows:

### `check`

Used to determine if the MOOSE webserver is currently listening. This doesn't necessarily mean that it is waiting for input.

Interact with this endpoint by a `GET` request to `/check`. On success, the status code will be 200.

### `waiting`

Used to determine of MOOSE is currently waiting within the `WebServerControl`, i.e., that `Control` is waiting for external input. The rest of the endpoints that follow require that the control is currently waiting for input.

Interact with this endpoint by a `GET` request to `/waiting` without any data. If the control is currently waiting, the response will be of type `application/json` of the form:

```language=json
{
  "waiting": True
  "execute_on_flag": "<EXEC_ON_FLAG>"
}
```

where `<EXEC_ON_FLAG>` is the current execution flag. If the control is not currently waiting, the response will be a response of type `application/json` of the form:

```language=json
{
  "waiting": False
}
```

This endpoint can be accessed via the [MooseControl](MooseControl/index.md optional=true) python utility via the following methods:

- `wait()`: Waits for the control to be waiting
- `getWaitingFlag()`: Gets the current flag that the control is waiting on, if any
- `isWaiting()`: Whether or not the control is currently waiting

### `get/postprocessor`

Used to obtain the value of a postprocessor. The control must be waiting in order to access this endpoint.

Interact with this endpoint by a `POST` request to `/get/postprocessor` with the following `application/json` data:

```language=json
{
  "name": "<NAME>"
}
```

where `<NAME>` is the name of the postprocessor whose value you wish to receive. The response will be of the form:

```language=json
{
  "value": <VALUE>
}
```

### `set/controllable`

Used to change a controllable parameter in the simulation. The control must be waiting in order to access this endpoint.

The following parameter types are currently supported by the endpoint:

- `bool`
- `Real`
- `std::string`
- `std::vector<Real>`
- `std::vector<std::string>`

These types can be extended by the `registerWebServerControl[Scalar/Vector][BoolNumberString]` registration methods in the source for the `WebServerControl`.

Interact with this endpoint by a `POST` request to `/set/controllable` with the following `application/json` data:

```language=json
{
  "name": "<NAME>",
  "type" "<TYPE>",
  "value": <VALUE>
}
```

where `<NAME>` is the string path to the controllable parameter, `<TYPE>` is the string version of the C++ parameter type, and `<VALUE>` is the value to set the parameter to. The type of `<VALUE>` depends on the controllable parameter type.

On success, the response will be empty with a status code of 201.

This endpoint can be accessed via the [MooseControl](MooseControl/index.md optional=true) python utility via the following methods:

- `setControllableBool()`: Sets a controllable `bool` parameter
- `setControllableReal()`: Sets a controllable `Real` parameter
- `setControllableVectorReal()`: Sets a controllable `std::vector<Real>` parameter
- `setControllableString()`: Sets a controllable `std::string` parameter
- `setControllableVectorString()`: Sets a controllable `std::vector<std::string>` parameter

### `continue`

Tells a waiting control to continue with the execution. The control must be waiting in order to access this endpoint.

Interact with this endpoint by a `GET` request to `/continue`. On success, the response will be empty with a status code of 200.

!syntax parameters /Controls/WebServerControl

!syntax inputs /Controls/WebServerControl

!syntax children /Controls/WebServerControl
