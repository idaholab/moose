# PNSFVEnergyTimeDerivative

## Description

This kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\epsilon \rho c_{pf} \frac{\partial T_f}{\partial t}}_{\textrm{PNSFVEnergyTimeDerivative}}
\end{equation}
for the fluid phase and
\begin{equation}
\underbrace{(1 - \epsilon) \rho_s c_{ps} \frac{\partial T_s}{\partial t}}_{\textrm{INSFVEnergyTimeDerivative}}
\end{equation}
for the solid phase, where $\epsilon$ is the porosity, $\rho_{f/s}$ the fluid/solid material density, $c_{pf/s}$ the fluid/solid specific heat and $T_{f/s}$ the fluid/solid temperature.

The variation of the kinetic energy is not considered in this kernel.

!syntax parameters /FVKernels/PNSFVEnergyTimeDerivative

!syntax inputs /FVKernels/PNSFVEnergyTimeDerivative

!syntax children /FVKernels/PNSFVEnergyTimeDerivative
