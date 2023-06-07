# PrandtlNumberAux

!syntax description /AuxKernels/PrandtlNumberAux

The Prandtl number $Pr$ is computed as:

!equation
Pr = \dfrac{c_p * \mu}{k}

where $c_p$ is the specific heat capacity, $\mu$ the dynamic viscosity and $k$ the thermal conductivity.

!alert note
The fluid properties are evaluated using the conserved variable set of specific volume and specific internal energy.
To use pressure and temperature, you may consider using a [FluidPropertiesMaterialPT.md] along with a
[ParsedMaterial.md] to define a Prandtl number material property.

!syntax parameters /AuxKernels/PrandtlNumberAux

!syntax inputs /AuxKernels/PrandtlNumberAux

!syntax children /AuxKernels/PrandtlNumberAux
