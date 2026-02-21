# LinearFVP1TemperatureSourceSink

## Description

The `LinearFVP1TemperatureSourceSink` kernel implements the source and the sink
terms for fluid energy equation from the interaction with the participating media
radiative heat transfer using the P1 formulation.
The term added reads as follows:

\begin{equation}
- \int_{V_C} \sigma_a (G - 4\sigma T^4) dV
\end{equation}

where:

-  $G$ is the incident radiation heat flux (SI units (W/m$^2$))
-  $\sigma_a$ is the absorption coefficient (SI units (1/m))
-  $\sigma$ is the Stefan-Boltzmann constant (SI units (W/m$^2$/K$^4$))
-  $T$ is the fluid medium temperature (SI units (K))

!alert note
This kernel assumes the solver variable is the medium temperature. It is not yet compatible with
enthalpy variable solvers.

## Example Input File Syntax

!listing test/tests/radiation_participating_media/rad_T_coupled_medium_1d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVP1TemperatureSourceSink

!syntax inputs /LinearFVKernels/LinearFVP1TemperatureSourceSink

!syntax children /LinearFVKernels/LinearFVP1TemperatureSourceSink
