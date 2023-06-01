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

!syntax parameters /FunctorMaterials/GeneralFunctorFluidProps

!syntax inputs /FunctorMaterials/GeneralFunctorFluidProps

!syntax children /FunctorMaterials/GeneralFunctorFluidProps
