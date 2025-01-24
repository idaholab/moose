# [Input Parameters](source/utils/InputParameters.md)

Every object in MOOSE includes a set of custom parameters within an `InputParameters` object that
is used to construct the object.

!---

### Required parameters

Some object parameters are required. MOOSE will error if they are not provided in the input file.

```bash
[Kernels]
  [diff]
    type = Diffusion
  []
[]
```

The example above will error because the `variable` parameter is missing.

!alert note
The `MOOSE VSCode plugin` will automatically create a `<required_parameter> =` line for each required
parameter when using the syntax auto-complete.

!---

### Optional Parameter(s) have a default

If the value of a parameter is standard, it does not need to be always included in the input file,
unless the user wants to change its value!
In the example below, the `block` parameter is not specified, which means the default (empty)
is applied.

```text
[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]
```

!alert note
The `MOOSE VSCode plugin` will automatically create a `<optional_parameter> = <default_value>` line for an optional parameter when using the auto-complete feature and selecting said parameter.

!---

### Coupled Variable

Various types of objects in MOOSE can be coupled to variables. MOOSE will then automatically
compute the local variable values when executing the object.

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

Input constant values may be restricted to a range of values.
This is enforced programmatically, and MOOSE will error if the user passes a value
out of the specified bounds.

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

## Supported types

MOOSE can support integer, float, vector, string, ... parameters.
However this is specified in each object, and types cannot be changed in the
input file.

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
- DataFileName
- VariableName
- FunctionName
- UserObjectName
- PostprocessorName
- MeshFileName
- OutFileName
- NonlinearVariableName
- AuxVariableName

!---

### Enumerations

MOOSE supports enumerations as parameters. An enumeration is a fixed list of options,
that the user may then select from. Defaults are supported for these enumerations.

In the example below, the `value_type` parameters can take `max` or `min` values, with a default
of `max`. If you specify `abc` to `value_type`, it will error.

!listing test/tests/postprocessors/element_extreme_value/element_extreme_value.i block=Postprocessors
