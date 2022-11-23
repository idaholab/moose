# PINSFVEnergyTimeDerivative

## Description

This kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\epsilon \frac{\partial \rho c_{pf} T_f}{\partial t}
\end{equation}
for the fluid phase and
\begin{equation}
(1 - \epsilon) \frac{\partial \rho_s c_{ps} T_s}{\partial t}
\end{equation}
for the solid phase, where $\epsilon$ is the porosity, $\rho_{f/s}$ the fluid/solid material density, $c_{pf/s}$ the fluid/solid specific heat and $T_{f/s}$ the fluid/solid temperature.

The time derivative of the density is ignored if [!param](/FVKernels/PINSFVEnergyTimeDerivative/drho_dt) is not provided. For incompressible flows, the former should not be provided.

The variation of the kinetic energy is not considered in this kernel.

!syntax parameters /FVKernels/PINSFVEnergyTimeDerivative

!syntax inputs /FVKernels/PINSFVEnergyTimeDerivative

!syntax children /FVKernels/PINSFVEnergyTimeDerivative
