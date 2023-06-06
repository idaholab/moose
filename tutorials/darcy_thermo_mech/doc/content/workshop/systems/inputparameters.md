# [Input Parameters](source/utils/InputParameters.md)

Every `MooseObject` includes a set of custom parameters within the `InputParameters` object that
is used to construct the object.

The `InputParameters` object for each object is created using the static `validParams` method,
which every class contains.

!---

## `validParams` Declaration

In the class declaration,
```cpp
public:
...
static InputParameters Convection::validParams();
```

!---

## `validParams` Definition

```cpp
InputParameters
Convection::validParams()
{
  InputParameters params = Kernel::validParams();  // Start with parent
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  params.addParam<Real>("coefficient", "Diffusion coefficient");
  return params;
}
```

!---

## `InputParameters` Object

!---

### Class Description

Class level documentation may be included within the source code using the `addClassDescription`
method.

```C++
params.addClassDescription("Use this method to provide a summary of the class being created...");
```

!---

### Optional Parameter(s)

Adds an input file parameter, of type `int`, that includes a default value of 1980.

```cpp
params.addParam<int>("year", 1980, "Provide the year you were born.");
```

Here the default is overriden by a user-provided value

```text
[UserObjects]
  [date_object]
    type = Date
    year = 1990
  []
[]
```

!---

### Required Parameter(s)

Adds an input file parameter, of type `int`, must be supplied in the input file.

```cpp
params.addRequiredParam<int>("month", "Provide the month you were born.");
```

```text
[UserObjects]
  [date_object]
    type = Date
    month = 6
  []
[]
```

!---

### Coupled Variable

Various types of objects in MOOSE support variable coupling, this is done using the
`addCoupledVar` method.

```cpp
params.addRequiredCoupledVar("temperature", "The temperature (C) of interest.");
params.addCoupledVar("pressure", 101.325, "The pressure (kPa) of the atmosphere.");
```

```text
[Variables]
  [P][]
  [T][]
[]

[UserObjects]
  [temp_pressure_check]
    type = CheckTemperatureAndPressure
    temperature = T
    pressure = P # if not provided a value of 101.325 would be used
  []
[]
```

!---

Within the input file it is possible to used a variable name or a constant value for a `coupledVar`
parameter.

```text
pressure = P
pressure = 42
```

!---

### Range Checked Parameters

Input constant values may be restricted to a range of values directly in the `validParams` function.
This can also be used for vectors of constants parameters!

!listing timesteppers/ConstantDT.C start=params.addRangeCheckedParam end=params.addClassDescription

Syntax: [warp.povusers.org/FunctionParser/fparser.html](http://warp.povusers.org/FunctionParser/fparser.html)


!---


## Documentation

Each application is capable of generating documentation from the `validParams` functions.

+Option 1+: Command line `--dump`

- `--dump [optional search string]`
- the search string may contain wildcard characters
- searches both block names and parameters

+Option 2+: Command line `--show-input` generates a tree based on your input file\\

+Option 3+: [mooseframework.org/syntax](syntax/index.md alternative=https://mooseframework.org/syntax)

!---

## C++ Types

Built-in types and std::vector are supported via template methods:

- `addParam<Real>("year", 1980, "The year you were born.");`
- `addParam<int>("count", 1, "doc");`
- `addParam<unsigned int>("another_num", "doc");`
- `addParam<std::vector<int> >("vec", "doc");`

Other supported parameter types include:

- `Point`
- `RealVectorValue`
- `RealTensorValue`
- `SubdomainID`
- `std::map<std::string, Real>`

!---

MOOSE uses a large number of string types to make `InputParameters` more context-aware. All of these types can
be treated just like strings, but will cause compile errors if mixed improperly in the template
functions.

- SubdomainName
- BoundaryName
- FileName
- VariableName
- FunctionName
- UserObjectName
- PostprocessorName
- MeshFileName
- OutFileName
- NonlinearVariableName
- AuxVariableName

For a complete list, see the instantiations at the bottom of framework/include/utils/MooseTypes.h.

!---

## MooseEnum

MOOSE includes a "smart" enum utility to overcome many of the deficiencies in the standard C++ enum
type. It works in both integer and string contexts and is self-checked for consistency.

```cpp
#include "MooseEnum.h"

// The valid options are specified in a space separated list.
// You can optionally supply the default value as a second argument.
// MooseEnums are case preserving but case-insensitive.
MooseEnum option_enum("first=1 second fourth=4", "second");

// Use in a string context
if (option_enum == "first")
  doSomething();

// Use in an integer context
switch (option_enum)
{
  case 1: ... break;
  case 2: ... break;
  case 4: ... break;
  default: ... ;
}
```

!---

## MooseEnum with InputParameters

Objects that have a specific set of named options should use a `MooseEnum` so that parsing and error
checking code can be omitted.

```cpp
InputParameters MyObject::validParams()
{
  InputParameters params = ParentObject::validParams();
  MooseEnum component("X Y Z");  // No default supplied
  params.addRequiredParam<MooseEnum>("component", component,
                                     "The X, Y, or Z component");
  return params;
}
```

If an invalid value is supplied, an error message is provided.

!---

## Multiple Value MooseEnums (MultiMooseEnum)

Operates the same way as `MooseEnum` but supports multiple ordered options.

```cpp
InputParameters MyObject::validParams()
{
  InputParameters params = ParentObject::validParams();
  MultiMooseEnum transforms("scale rotate translate");
  params.addRequiredParam<MultiMooseEnum>("transforms", transforms,
                                          "The transforms to perform");
  return params;
}
```
