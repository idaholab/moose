# PINSFVEnergyAmbientConvection

## Description

This kernel implements a volumetric convection heat transfer term for the fluid and solid energy equation.
It adds the following terms to the RHS of the equations:

For the fluid energy equation:
\begin{equation}
h (T_s - T_f)
\end{equation}

For the solid energy equation:
\begin{equation}
h (T_f - T_s)
\end{equation}
where $h$ is the heat transfer coefficient, $T_s$ the solid temperature and $T_f$ the fluid temperature.

!syntax parameters /FVKernels/PINSFVEnergyAmbientConvection

!syntax inputs /FVKernels/PINSFVEnergyAmbientConvection

!syntax children /FVKernels/PINSFVEnergyAmbientConvection
