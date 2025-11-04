# HeatSource

## Description

The `HeatSource` kernel implements a volumetric heat source/sink forcing term. The strong form is

\begin{equation}
\underbrace{-\dot{q}}_{\textrm{HeatSource}} + \text{other kernels} = 0 \in \Omega
\end{equation}

where $\dot{q}$ is the volumetric heat source. The corresponding weak form,
in inner-product notation, is

\begin{equation}
R_i(u_h)=(\psi_i, -\dot{q})\quad\forall \psi_i,
\end{equation}

where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

A slightly more general version of this same kernel can be found in [BodyForce](BodyForce.md), which you can equivalently use instead of `HeatSource`.

## Example Input File Syntax

!listing modules/heat_transfer/tutorials/introduction/therm_step03a.i
  block=Kernels

!syntax parameters /Kernels/HeatSource

!syntax inputs /Kernels/HeatSource

!syntax children /Kernels/HeatSource
