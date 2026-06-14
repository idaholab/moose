# HeatConduction

## Description

`HeatConduction` implements the diffusion kernel in the thermal energy conservation equation, with a material property for the thermal conductivity. The strong form is

\begin{equation}
\underbrace{-\nabla\cdot(k\nabla T)}_{\textrm{HeatConduction}} + \text{other kernels} = 0 \in \Omega
\end{equation}

where $k$ is the thermal conductivity and $T$ is
the variable (temperature). The corresponding weak form,
in inner-product notation, is

\begin{equation}
R_i(u_h)=(\nabla\psi_i, k\nabla u_h)\quad\forall \psi_i,
\end{equation}

where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The [!param](/Kernels/HeatConduction/thermal_conductivity)
parameter is used to define the material property name
which contains the thermal conductivity.
The Jacobian will account for partial derivatives of the thermal conductivity
with respect to the unknown variable if the [!param](/Kernels/HeatConduction/thermal_conductivity_dT) property
name is provided. These particular defaults for these parameters
are the names used by [HeatConductionMaterial](HeatConductionMaterial.md),
though you can also define these materials using other [Material](Materials/index.md) objects.

## Example Input File Syntax

The case below demonstrates the use of `HeatConduction` where the thermal conductivity is defined by a `HeatConductionMaterial`.

!listing modules/heat_transfer/tutorials/introduction/therm_step02.i
  start=Kernels
  end=BCs

The case below instead demonstrates the use of `HeatConduction` where the thermal conductivity is defined by a [ParsedMaterial](ParsedMaterial.md)

!listing modules/heat_transfer/test/tests/code_verification/spherical_test_no2.i
  start=Kernels
  end=Executioner
  remove=BCs

!syntax parameters /Kernels/HeatConduction

!syntax inputs /Kernels/HeatConduction

!syntax children /Kernels/HeatConduction
