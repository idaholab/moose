# PorousFlowPropertyAux
!syntax description /AuxKernels/PorousFlowPropertyAux

This `AuxKernel` provides simplified access to fluid and material properties. The
following properties are available using the `property` input parameter:

- `pressure`
- `saturation`
- `temperature`
- `density`
- `viscosity`
- `mass_fraction`
- `relperm`
- `enthalpy`
- `internal_energy`

The fluid phase and fluid component are specified in the `phase` and `fluid_component`
input parameters, respectively.

!!! note:
    As this `AuxKernel` uses material properties, only elemental (`Monomial`) `AuxVariables`
    can be used.

!syntax parameters /AuxKernels/PorousFlowPropertyAux

!syntax inputs /AuxKernels/PorousFlowPropertyAux

!syntax children /AuxKernels/PorousFlowPropertyAux
