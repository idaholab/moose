# HeatConduction

## Description

`HeatConduction` implements the diffusion kernel in the thermal energy conservation equation, with a material property for the diffusion coefficient. The strong form is

\begin{equation}
\underbrace{-\nabla\cdot(k\nabla T)}_{\textrm{HeatConduction}} + \text{other kernels} = 0 \in \Omega
\end{equation}

where $k$ is the diffusion coefficient (thermal conductivity) and $T$ is
the variable (temperature). The corresponding weak form,
in inner-product notation, is

\begin{equation}
R_i(u_h)=(\nabla\psi_i, k\nabla u_h)\quad\forall \psi_i,
\end{equation}

where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The diffusion coefficient is specified with a material property; the
[!param](/Kernels/HeatConduction/diffusion_coefficient)
parameter is used to define the material property name
which contains the diffusion coefficient.
The Jacobian will account for partial derivatives of the diffusion coefficient
with respect to the unknown variable if the [!param](/Kernels/HeatConduction/diffusion_coefficient_dT) property
name is provided. These particular defaults for these parameters
are the names used by [HeatConductionMaterial](HeatConductionMaterial.md),
though you can also define these materials using other [Material](Materials/index.md) objects.

## Example Input File Syntax

The case below demonstrates the use of `HeatConduction` where the diffusion
coefficient (thermal conductivity) is defined by a `HeatConductionMaterial`.

!listing modules/heat_transfer/tutorials/introduction/therm_step02.i
  start=Kernels
  end=BCs

The case below instead demonstrates the use of `HeatConduction` where the
diffusion coefficient (thermal conductivity) is defined by a [ParsedMaterial](ParsedMaterial.md)

!listing modules/heat_transfer/test/tests/code_verification/spherical_test_no2.i
  start=Kernels
  end=Executioner
  remove=BCs

!syntax parameters /Kernels/HeatConduction

!syntax inputs /Kernels/HeatConduction

!syntax children /Kernels/HeatConduction
