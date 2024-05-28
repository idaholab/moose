# FVThermalRadiationSourceSink

## Description

The `FVThermalRadiationSourceSink` kernel implements source and the sink
terms for radiation heat transfer.
The term added reads as follows:

\begin{equation}
- \int_{\Omega_C} \kappa (\sigma T_{rad}^4 - G) dV
\end{equation}

where:
-  $G$ is the radiation heat flux (SI units $W/m^2$)
-  $\kappa$ is the oppacity (SI units $1/m$)
-  $\sigma$ is the Stefan-Boltzmann constant
-  $T_{rad}$ is the rqadiation temperature

!syntax parameters /FVKernels/FVThermalRadiationSourceSink

!syntax inputs /FVKernels/FVThermalRadiationSourceSink

!syntax children /FVKernels/FVThermalRadiationSourceSink
