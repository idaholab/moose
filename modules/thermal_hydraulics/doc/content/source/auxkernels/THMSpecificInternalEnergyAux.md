# THMSpecificInternalEnergyAux

!syntax description /AuxKernels/THMSpecificInternalEnergyAux

The specific internal energy $e$ is computed as

!equation
e = \dfrac{ \rho E - 0.5 * \rho u * \rho u}{\rho \rho}

where $\rho$ the fluid density, $\rho u$ the momentum and
$\rho E$ the product of the internal energy by the density.

!alert note
$\rho E$, $\rho u$ and $\rho$ are variables usually defined by the [Components](syntax/Components/index.md).

!syntax parameters /AuxKernels/THMSpecificInternalEnergyAux

!syntax inputs /AuxKernels/THMSpecificInternalEnergyAux

!syntax children /AuxKernels/THMSpecificInternalEnergyAux
