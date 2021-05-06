# WaveHeightAuxKernel

!syntax description /AuxKernels/WaveHeightAuxKernel

## Description

This auxkernel calculates the wave heights from fluid pressures. This is done as
follows:

\begin{equation}
    \label{eqn:Wave1}
    d_w = \frac{p}{\rho g}
\end{equation}

where, $p$ is the fluid pressure, $g$ is the acceleration due to gravity, $\rho$ is
the fluid density, and $d_w$ is the wave height.

!syntax parameters /AuxKernels/WaveHeightAuxKernel

!syntax inputs /AuxKernels/WaveHeightAuxKernel

!syntax children /AuxKernels/WaveHeightAuxKernel
