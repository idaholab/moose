# InterfaceValueTools

## Description

Basic utility file to compute a single quantity given values on both side of an interface. The value to compute is selected via the input parameter
interface_value_type. This function is not registered and as such should only be used for simplifying the development of other objects.
An example of an object using this utility is  [InterfaceAverageVariableValuePostprocessor](/InterfaceAverageVariableValuePostprocessor.md).

Available outputs are:
* average: (value_m-value_s)/2
* jump_primary_minus_secondary: value_m-value_s
* jump_secondary_minus_primary: value_s-value_m
* jump_abs: abs(value_m-value_s)
* primary: value_m
* secondary: value_s

where value_m is the value on the primary side of the interface (e.g. where the boundary is defined) and value_s is the value on the secondary side of the interface.
