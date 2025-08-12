# GapConductance

!syntax description /Materials/GapConductance

This material is automatically created by the [ThermalContactAction.md] when the gap conductance
has not been specified by the user. It captures both conduction across the gap and radiation.
This material takes care of computing both the conductance and the derivative of conductance with
regards to temperature.

## Radiative term

Gap conductance due to radiation is based on the diffusion approximation.
Between surface $1$ and $2$, we define the heat flux as:

!equation
q_{12} = \sigma*Fe*(T_1^4 - T_2^4) ~ h_r(T_1 - T_2)

where $\sigma$ is the Stefan-Boltzmann constant, $Fe$ is an emissivity
function, $T_1$ and $T_2$ are the temperatures of the two surfaces, and
$h_r$ is the radiant gap conductance. Solving for $h_r$,

!equation
h_r = \sigma*Fe*(T_1^4 - T_2^4) / (T_1 - T_2)

which can be factored to give:

!equation
h_r = \sigma*Fe*(T_1^2 + T_2^2) * (T_1 + T_2)

Assuming the gap is between infinite parallel planes, the emissivity
function is given by:

!equation
Fe = 1 / (1/e_1 + 1/e_2 - 1)

For cylinders and spheres, see [!citep](incropera2002).
$r_1$ is the radius of surface 1, the primary surface, and
$r_2$ the radius of the secondary surface.
For cylinders:

!equation
Fe = 1 / (1/e_1 + (1/e_2 - 1) * (r_1/r_2))

!equation
q_{21} = -q_{12} * (r_1/r_2)

For spheres:

!equation
Fe = 1 / (1/e_1 + (1/e_2 - 1) * (r_1/r_2)^2)

!equation
q_{21} = -q_{12} * (r_1/r_2)^2

!syntax parameters /Materials/GapConductance

!syntax inputs /Materials/GapConductance

!syntax children /Materials/GapConductance
