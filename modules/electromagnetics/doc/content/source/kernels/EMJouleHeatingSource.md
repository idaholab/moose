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
