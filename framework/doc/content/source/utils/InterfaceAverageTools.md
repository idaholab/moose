# InterfaceAverageTools

## Description

Basic utility file to compute a single quantity given values on both side of an interface. The value to compute is selected via the input parameter
average_type. This function is not registered and as such should only be used for simplifying the development of other objects.
An example of an object using this utility is  [InterfaceIntegralVariableAveragePostprocessor](/InterfaceIntegralVariableAveragePostprocessor.md).

Available outputs are:
* average: (value_m-value_s)/2
* jump_master_minus_slave: value_m-value_s
* jump_slave_minus_master: value_s-value_m
* jump_abs: abs(value_m-value_s)
* master: value_m
* slave: value_s

where value_m is the value on the master side of the interface (e.g. where the boundary is defined) and value_s is the value on the slave side of the interface.
