!alert! warning
This kernel will be deprecated in the near future
(10/01/2025) in favor of exclusively using the [JouleHeatingHeatGeneratedAux](source/auxkernels/JouleHeatingHeatGeneratedAux.md optional=true)
object within the [Heat Transfer](modules/heat_transfer/index.md optional=True) module, because `JouleHeatingHeatGeneratedAux` can calculate both electrostatic
and electromagnetic Joule heating. For more information, please see
[JouleHeatingHeatGeneratedAux](source/auxkernels/JouleHeatingHeatGeneratedAux.md optional=true).
!alert-end!

# EMJouleHeatingHeatGeneratedAux

!syntax description /AuxKernels/EMJouleHeatingHeatGeneratedAux

## Overview

!style halign=left
The EMJouleHeatingHeatGeneratedAux object calculates the heating term imparted to the medium based on the conduction current. The term is defined as:

\begin{equation}
  0.5 * Re \left(\sigma \; \vec{E} \cdot \vec{E}^{*} \right)
\end{equation}

where

- $\sigma$ is the conductivity of the medium,
- $\vec{E}$ is the electric field, and
- $\vec{E}^{*}$ is the complex conjugate of the electric field.

## Example Input File Syntax

!listing /auxkernels/heating/aux_microwave_heating.i block=AuxKernels/aux_microwave_heating

!syntax parameters /AuxKernels/EMJouleHeatingHeatGeneratedAux

!syntax inputs /AuxKernels/EMJouleHeatingHeatGeneratedAux

!syntax children /AuxKernels/EMJouleHeatingHeatGeneratedAux
