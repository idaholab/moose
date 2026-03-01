# LinearFVP1RadiationSourceSink

## Description

The `LinearFVP1RadiationSourceSink` kernel implements the source and the sink
terms for radiation heat transfer using the P1 formulation for the incident radiation.
The term added reads as follows:

\begin{equation}
- \int_{V_C} \sigma_a (4\sigma T_{rad}^4 - G) dV
\end{equation}

where:

-  $G$ is the incident radiation heat flux (SI units (W/m$^2$))
-  $\sigma_a$ is the absorption coefficient (SI units (1/m))
-  $\sigma$ is the Stefan-Boltzmann constant (SI units (W/m$^2$/K$^4$))
-  $T_{rad}$ is the radiation temperature (SI units (K))

## Example Input File Syntax

!listing test/tests/radiation_participating_media/rad_isothermal_medium_1d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVP1RadiationSourceSink

!syntax inputs /LinearFVKernels/LinearFVP1RadiationSourceSink

!syntax children /LinearFVKernels/LinearFVP1RadiationSourceSink
