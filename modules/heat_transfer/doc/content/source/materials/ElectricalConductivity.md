# ElectricalConductivity

!syntax description /Materials/ElectricalConductivity

## Overview

!alert warning title=Assumptions
It is assumed here that resistivity varies linearly with temperature. This is
acceptable for small variations in temperature, but may break down if there are
larger temperature variations.

Resistivity ($\rho_{elec}$) with respect to temperature $T$ is defined as

\begin{equation}
  \rho_{elec}(T) = \rho_{ref} [1 + \alpha(T - T_{ref})]
\end{equation}

where $\rho_{ref}$ is the reference resistivity of the material, $\alpha$ is the
temperature coefficient of the material, and $T_{ref}$ is the reference
temperature.

Electrical conductivity can then be calculated via

\begin{equation}
  \sigma_{elec} = \frac{1}{\rho_{elec}}
\end{equation}

!alert note title=Default parameter values
The defaults used for the parameters in this object are taken from published
values for Copper.

## Example Input File Syntax

An example of how to use `ElectricalConductivity` can be found in the
heat conduction module test `transient_jouleheating.i`.

!listing modules/heat_conduction/test/tests/joule_heating/transient_jouleheating.i block=Materials/sigma

!syntax parameters /Materials/ElectricalConductivity

!syntax inputs /Materials/ElectricalConductivity

!syntax children /Materials/ElectricalConductivity
