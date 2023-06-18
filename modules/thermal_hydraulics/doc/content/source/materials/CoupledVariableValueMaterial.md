# CoupledVariableValueMaterial

!syntax description /Materials/CoupledVariableValueMaterial

The value of the material property, named using the
[!param](/Materials/CoupledVariableValueMaterial/prop_name) parameter, is then simply:

!equation
p = v

at every quadrature point, with $p$ the material property and $v$ the field variable.

The AD version of this object is the `ADCoupledVariableValueMaterial`.

For a more general capability than simply storing the value of a variable in a
material property, please consider the [ParsedMaterial.md].

!syntax parameters /Materials/CoupledVariableValueMaterial

!syntax inputs /Materials/CoupledVariableValueMaterial

!syntax children /Materials/CoupledVariableValueMaterial
