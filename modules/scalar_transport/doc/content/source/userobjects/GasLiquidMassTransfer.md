# GasLiquidMassTransfer

!syntax description /UserObjects/GasLiquidMassTransfer

## Description

This object implements two models for estimating the mass transfer coefficient of a flowing
system.  The first model is the one described by C.R. Wilke, P. Chang, ``Correlation of diffusion
coefficients in dilute solutions'', AICHE J., 1955, 1(2) 264-270.

The second model is the famous Stokes-Einstein relationship, which provides the diffusion of a spherical
partical through a liquid with a low Reynolds number. The relationship is as follows:

\begin{equation}
   D = \frac{k_BT} / (6\pi\muR)
\end{equation}

Here, $D$ is the diffusivity being calculated, $k_B$ is the Boltzmann constant, $T$ is the temperature
of the fluid, $\mu$ is the dynamic viscosity, and $R$ is the particle radius.

!syntax parameters /UserObjects/GasLiquidMassTransfer

!syntax inputs /UserObjects/GasLiquidMassTransfer

!syntax children /UserObjects/GasLiquidMassTransfer
