!alert! warning
This kernel will be deprecated in the near future
(10/01/2025) in favor of exclusively using the [ADJouleHeatingSource](ADJouleHeatingSource.md), because
`ADJouleHeatingSource` can calculate both electrostatic and electromagnetic Joule heating.
For more information, please see [ADJouleHeatingSource](ADJouleHeatingSource.md).
!alert-end!

# JouleHeatingSource

!syntax description /Kernels/JouleHeatingSource

## Overview

The strong form for `JouleHeatingSource` is defined as

\begin{equation}
q = \mathbf{J} \cdot \mathbf{E} = (\sigma_{elec} \nabla \phi) \cdot \nabla \phi
\end{equation}

where $\phi$ is the electrostatic potential and $\sigma_{elec}$ is the
electrical conductivity of the material. $\sigma_{elec}$ can either be an
`ADMaterial` or a traditional `Material`. Within the heat transfer module,
this default could currently be provided by
[ElectricalConductivity](ElectricalConductivity.md) for general problems or
[SemiconductorLinearConductivity](SemiconductorLinearConductivity.md) for more
specific problems related to semiconductors.

This class inherits from the [HeatSource](HeatSource.md) class via
[DerivativeMaterialInterface](DerivativeMaterialInterface.md) and
[JvarMapInterface](JvarMapInterface.md).

## Example Input File Syntax

An example of how to use `JouleHeatingSource` can be found in the
heat transfer module test `transient_jouleheating.i`.

!listing modules/heat_transfer/test/tests/joule_heating/transient_jouleheating.i block=Kernels/HeatSrc

!syntax parameters /Kernels/JouleHeatingSource

!syntax inputs /Kernels/JouleHeatingSource

!syntax children /Kernels/JouleHeatingSource
