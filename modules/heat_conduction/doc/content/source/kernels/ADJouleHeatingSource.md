# ADJouleHeatingSource

!syntax description /Kernels/ADJouleHeatingSource

## Overview

`ADJouleHeatingSource` is the implementation of a heat source corresponding to
electric Joule heating, as in [JouleHeatingSource](JouleHeatingSource.md), within
the framework of automatic differentiation.

The strong form for `ADJouleHeatingSource` is defined as

\begin{equation}
q = \mathbf{J} \cdot \mathbf{E} = (\sigma_{elec} \nabla \phi) \cdot \nabla \phi
\end{equation}

where $\phi$ is the electrostatic potential and $\sigma_{elec}$ is the
electrical conductivity of the material. $\sigma_{elec}$ is defined as an
`ADMaterialProperty`. Within the heat conduction module, this property could
currently be provided by [ADElectricalConductivity](ADElectricalConductivity.md).

This class inherits from the [ADKernelValue](Kernel.md) class.

## Example Input File Syntax

An example of how to use `ADJouleHeatingSource` can be found in the
heat conduction module test `transient_ad_jouleheating.i`.

!listing modules/heat_conduction/test/tests/joule_heating/transient_ad_jouleheating.i block=Kernels/HeatSrc

!syntax parameters /Kernels/ADJouleHeatingSource

!syntax inputs /Kernels/ADJouleHeatingSource

!syntax children /Kernels/ADJouleHeatingSource
