# FluidProperties System

## Overview

`FluidProperties` objects define interfaces for computing thermodynamic
properties of fluids (liquids and gases).

## Interfaces

Because two independent, intensive thermodynamic properties define a
thermodynamic state, most interfaces in these objects are of the form $f(a,b)$
where $f$ is the desired thermodynamic property and $a$ and $b$ are independent,
intensive thermodynamic properties that define the thermodynamic state. The
corresponding function name is `f_from_a_b`. See the following table for the
list of thermodynamic properties and their corresponding names in the interfaces:

| Name | Symbol | Description |
| - | - | - |
| `beta` | $\beta$ | Volumetric expansion coefficient |
| `c` | $c$ | Speed of sound |
| `cp` | $c_p$ | Isobaric specific heat capacity |
| `cv` | $c_v$ | Isochoric specific heat capacity |
| `e` | $e$ | Specific internal energy |
| `g` | $g$ | Gibbs free energy |
| `gamma` | $\gamma=\frac{c_p}{c_v}$ | Ratio of specific heats |
| `h` | $h$ | Specific enthalpy |
| `k` | $k$ | Thermal conductivity |
| `mu` | $\mu$ | Dynamic viscosity |
| `p` | $p$ | Pressure |
| `rho` | $\rho$ | Density |
| `s` | $s$ | Specific entropy |
| `T` | $T$ | Temperature |
| `v` | $v$ | Specific volume |

!syntax list /Modules/FluidProperties objects=True actions=False subsystems=False

!syntax list /Modules/FluidProperties objects=False actions=False subsystems=True

!syntax list /Modules/FluidProperties objects=False actions=True subsystems=False
