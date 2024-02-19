# GeneralFunctorFluidProps

!syntax description /FunctorMaterials/GeneralFunctorFluidProps

## Overview

This object uses a `SinglePhaseFluidProperties` derived-object to compute the following properties:

- specific heat at constant volume, $c_v$
- specific heat at constant pressure, $c_p$
- dynamic viscosity, $\mu$
- thermal conductivity, $k$
- Prandtl number, $\text{Pr}$
- pore/particle Reynolds number $\text{Re}$
- hydraulic Reynolds number $\text{Re}_h$
- interstitial Reynolds number $\text{Re}_i$


the time derivatives of the:

- specific heat at constant pressure, $c_p$
- density $\rho$


and the pressure and temperature derivatives of the:

- specific heat at constant pressure, $c_p$
- density $\rho$
- dynamic viscosity, $\mu$
- thermal conductivity, $k$
- Prandtl number, $\text{Pr}$
- pore Reynolds number $\text{Re}$

!alert note
In order to use this with some fluid properties that do not compute the AD version of the density
derivatives, such as the Spline Base Table Lookup fluid properties, you can use the
[!param](/FunctorMaterials/GeneralFunctorFluidProps/neglect_derivatives_of_density_time_derivative)
to neglect the derivatives with regards to the nonlinear variables (usually pressure, temperature)
of the time derivative of the density.

!syntax parameters /FunctorMaterials/GeneralFunctorFluidProps

!syntax inputs /FunctorMaterials/GeneralFunctorFluidProps

!syntax children /FunctorMaterials/GeneralFunctorFluidProps
