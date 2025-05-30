!alert! warning
This kernel will be deprecated in the near future
(10/01/2025) in favor of exclusively using the [ADJouleHeatingSource](source/kernels/ADJouleHeatingSource.md optional=true)
within the [Heat Transfer](modules/heat_transfer/index.md optional=True) module, because `ADJouleHeatingSource` can calculate both electrostatic
and electromagnetic Joule heating. For more information, please see
[ADJouleHeatingSource](source/kernels/ADJouleHeatingSource.md optional=true).
!alert-end!

# EMJouleHeatingSource

!syntax description /Kernels/EMJouleHeatingSource

## Overview

!style halign=left
The EMJouleHeatingSource object implements a heating term imparted to the medium based on the conduction current. The term is defined as:

\begin{equation}
  -0.5 * Re \left(\sigma \; \vec{E} \cdot \vec{E}^{*} \right) * \text{Scaling}
\end{equation}

where

- $\sigma$ is the conductivity of the medium,
- $\vec{E}$ is the electric field
- $\vec{E}^{*}$ is the complex conjugate of the electric field,
- and $\text{Scaling}$ is a scaling factor (usually used to convert the units of the heating term).


## Example Input File Syntax

!listing vector_helmholtz/microwave_heating.i block=Kernels/microwave_heating

!syntax parameters /Kernels/EMJouleHeatingSource

!syntax inputs /Kernels/EMJouleHeatingSource

!syntax children /Kernels/EMJouleHeatingSource
