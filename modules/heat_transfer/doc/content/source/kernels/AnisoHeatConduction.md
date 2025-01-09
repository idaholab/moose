# AnisoHeatConduction

## Description

`AnisoHeatConduction` implements the diffusion kernel in the thermal energy conservation equation, with an anisotropic material property for the thermal conductivity.
The strong form is

\begin{equation}
\underbrace{-\nabla\cdot(\mathbf{k}\nabla T)}_{\textrm{AnisoHeatConduction}} + \text{other kernels} = 0 \in \Omega
\end{equation}

where $\mathbf{k}$ is a tensor thermal conductivity (nine components) and $T$ is
temperature. The corresponding weak form,
in inner-product notation, is

\begin{equation}
R_i(u_h)=(\nabla\psi_i, \mathbf{k}\nabla u_h)\quad\forall \psi_i,
\end{equation}

where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The thermal conductivity is specified with a material property, [!param](/Kernels/AnisoHeatConduction/thermal_conductivity).

## Example Input File Syntax

The case below demonstrates the use of `AnisoHeatConduction` where the diffusion
coefficient (thermal conductivity) is defined by an [AnisoHeatConductionMaterial](AnisoHeatConductionMaterial.md).

!listing modules/heat_transfer/test/tests/heat_conduction_ortho/heat_conduction_ortho.i
  start=Kernels
  end=Executioner
  remove=BCs

!syntax parameters /Kernels/AnisoHeatConduction

!syntax inputs /Kernels/AnisoHeatConduction

!syntax children /Kernels/AnisoHeatConduction
