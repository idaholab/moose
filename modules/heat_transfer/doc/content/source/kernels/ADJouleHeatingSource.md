# ADJouleHeatingSource

!syntax description /Kernels/ADJouleHeatingSource

## Overview

`ADJouleHeatingSource` is the implementation of a heat source corresponding to
electric Joule heating ($Q$) within the framework of automatic differentiation.
The residual is provided by the [ElectromagneticHeatingMaterial](ElectromagneticHeatingMaterial.md)
material object and can be structured in the following formulations:

Time domain formulation:
\begin{equation}
  Q = (\sigma_{elec} E) \cdot E
\end{equation}

- where $E$ is the time domain electric field,
- and $\sigma_{elec}$ is th electrical conductivity.

Frequency domain formulation:
\begin{equation}
  Q = 0.5 Re( \sigma_{elec} E \cdot E^* )
\end{equation}

- where $E$ is the real component of the frequency domain electric field,
- and $E^*$ is the complex conjugate of the electric field.

$\sigma_{elec}$ is defined as an
`ADMaterialProperty`. Within the [heat transfer](modules/heat_transfer/index.md) module, this property could
currently be provided by [ADElectricalConductivity](ADElectricalConductivity.md).

In the case of the time domain formulation, the electric field can be
defined using an electromagnetic solver (like the [electromagnetics module](modules/electromagnetics/index.md optional=true)) or using the electrostatic
approximation, where:
\begin{equation}
  E = \nabla V
\end{equation}

- where $V$ is the electrostatic potential

This class inherits from the [ADKernelValue](Kernel.md) class.

## Example Input File Syntax

An example of how to use `ADJouleHeatingSource` can be found in the
heat transfer module test `transient_ad_jouleheating.i`.

!listing modules/heat_transfer/test/tests/joule_heating/transient_ad_jouleheating.i block=Kernels/HeatSrc

!syntax parameters /Kernels/ADJouleHeatingSource

!syntax inputs /Kernels/ADJouleHeatingSource

!syntax children /Kernels/ADJouleHeatingSource
