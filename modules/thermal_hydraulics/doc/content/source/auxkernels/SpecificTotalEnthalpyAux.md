# SpecificTotalEnthalpyAux

!syntax description /AuxKernels/SpecificTotalEnthalpyAux

The specific total enthalpy $h$ is computed as

!equation
h = \dfrac{ \rho E A + \alpha P A }{\rho A}

where $\rho$ the fluid density, $A$ the local component area, $P$ the pressure and
$\rho E A$ the product of the internal energy by the density by the local component area.

!alert note
$\rho E A$, $\rho A$ and $\rho$ are variables usually defined by the [Components](syntax/Components/index.md).

!syntax parameters /AuxKernels/SpecificTotalEnthalpyAux

!syntax inputs /AuxKernels/SpecificTotalEnthalpyAux

!syntax children /AuxKernels/SpecificTotalEnthalpyAux
