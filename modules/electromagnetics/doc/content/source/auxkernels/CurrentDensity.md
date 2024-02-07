# CurrentDensity / ADCurrentDensity

!syntax description /AuxKernels/CurrentDensity

## Overview

`CurrentDensity` and `ADCurrentDensity` allow for the calculation of the electric
current density given by

\begin{equation}
  \vec{J} = \sigma \vec{E}
\end{equation}

where

- $\vec{J}$ is the current density (SI units of A/m$^2$),
- $\sigma$ is the electrical conductivity (SI units of S/m), and
- $\vec{E}$ is the electric field (SI units of V/m).

The electric field can be determined either directly via an electromagnetic field
calculation (in which case, the [!param](/AuxKernels/CurrentDensity/electrostatic) 
parameter should be set to `false`), or calculated via the electrostatic potential 
$V$, where

\begin{equation}
  \vec{E} = -\nabla V.
\end{equation}

Note that calculation via the electrostatic potential is the default behavior.
MOOSE errors will be thrown if the inappropriate coupled variable is provided,
given the setting for the [!param](/AuxKernels/CurrentDensity/electrostatic) 
boolean parameter.

Additionally, note that the electrical conductivity $\sigma$ must be provided
as a material property for the block(s) this kernel is being applied to. The
user can use, e.g., [GenericConstantMaterial.md], [GenericFunctionMaterial.md],
or one of their AD counterparts. The user must also set the
[!param](/Materials/GenericConstantMaterial/prop_names) parameter of the chosen
object to `electrical_conductivity`.

## Example Input File Syntax

!listing /current_density.i block=AuxKernels/current_density

!syntax parameters /AuxKernels/CurrentDensity

!syntax inputs /AuxKernels/CurrentDensity

!syntax children /AuxKernels/CurrentDensity
