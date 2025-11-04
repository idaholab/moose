# HeatConductionTimeDerivative

## Description

`HeatConductionTimeDerivative` implements the time derivative term in the
thermal energy conservation equation. The strong form is

\begin{equation}
\label{eq:hctd}
\underbrace{\rho C_p\frac{\partial T}{\partial t}}_{\textrm{HeatConductionTimeDerivative}} + \text{other kernels} = 0 \in \Omega
\end{equation}

where $\rho$ is density, $C_p$ is the volumetric isobaric specific heat, and $T$ is
temperature.

!alert note
This strong form does *not* assume that $\rho$ or $C_p$ are constant. Eq. [eq:hctd] is the rigorously-derived form which can be used for $\rho$ and $C_p$ which are not constant.

The corresponding weak form using inner-product notation is

\begin{equation}
R_i(u_h) = (\psi_i, \rho c_p\frac{\partial u_h}{\partial t}) \quad \forall \psi_i,
\end{equation}

where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The density and specific heat are specified with material properties,
and the [!param](/Kernels/HeatConductionTimeDerivative/density_name) and
[!param](/Kernels/HeatConductionTimeDerivative/specific_heat) parameters are used to define the material property
name providing those properties.
The Jacobian will account for partial derivatives of $\rho$ and $C_p$
with respect to the unknown variable if the [!param](/Kernels/HeatConductionTimeDerivative/density_name_dT) and [!param](/Kernels/HeatConductionTimeDerivative/specific_heat_dT) property
names are also provided.

See also [/HeatCapacityConductionTimeDerivative.md] and [/SpecificHeatConductionTimeDerivative.md].

## Example Input File Syntax

The case below demonstrates the use of `HeatConductionTimeDerivative` where the
density and specific heat are defined by a [HeatConductionMaterial](/HeatConductionMaterial.md) (for specific heat) and a [ParsedMaterial](ParsedMaterial.md) for density.

!listing modules/heat_transfer/test/tests/transient_heat/transient_heat_derivatives.i
  start=Kernels
  end=Executioner
  remove=BCs

!syntax parameters /Kernels/HeatConductionTimeDerivative

!syntax inputs /Kernels/HeatConductionTimeDerivative

!syntax children /Kernels/HeatConductionTimeDerivative
