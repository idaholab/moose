# WebServerControl

The `WebServerControl` object is designed to allow an external process to control a MOOSE solve.
It works by starting up a webserver on the chosen `port` and exposing a REST API.
As with all REST APIs the input and output are both JSON.  Just use whatever REST API you like best
from your language (for Python `requests` is excellent).

## API

The `WebServerControl` presents four REST endpoints in order to control the solve:

### `waiting`

`get` this endpoint to find out if MOOSE is currently waiting within the WebServerControl.  i.e. that the `Control` is waiting for external input.

This endpoint returns a JSON message of the following format:

```language=json
{
  waiting: True/False
}
```

`True` means that MOOSE is waiting (i.e. you should take action).  `False` means that it is not.

### `set_controllable`

`post` to this endpoint to set the value of the parameter that is being controlled.
This endpoint expects a JSON message of the following format:

```language=json
{
  value: [double]
}
```

Obviously this only allows you to set floating point values for now.  More will be added later

### `get_pp`

`post` to this endpoint to get the value of any `Postprocessor` in the system
This endpoint expects to receive a JSON message of the following format:

```language=json
{
  pp: [postprocessorname]
}
```

And returns a JSON message like this that contains the value of the postprocessor:

```language=json
{
  value: [double]
}
```

### `continue`

`post` to this to tell MOOSE to continue execution.

## Example

Consider a simulation that solves the diffusion equation, where the Laplacian term has a
coefficient, but the coefficient is defined as a constant input parameter ("coef"). For some
reason, it is desired to control this coefficient from an external python script.

The `WebServerControl` object is designed for this purpose as shown below.

!listing test/tests/controls/web_server_control/web_server_control.i block=Controls caption=Control block demonstrating the use of the `WebServerControl` object.

Notice that the "parameter" input parameter is expecting a parameter name which can be defined
in various forms.

For a discussion on the naming of objects and parameters see
[Object and Parameter Names](syntax/Controls/index.md#object-and-parameter-names) section.

## External Control Example

As an example of how you can control MOOSE using this object, see the `controller.py` from the test for this object as shown below:

!listing test/tests/controls/web_server_control/controller.py caption=Demonstrates the control of the `WebServerControl` object.

!syntax parameters /Controls/WebServerControl

!syntax inputs /Controls/WebServerControl

!syntax children /Controls/WebServerControl
