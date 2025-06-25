# (AD)ChurchillChuHTCFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes a heat transfer coefficient $h$ due to natural convection heat between a long, horizontal, cylindrical, heated solid surface and a fluid using [functors](/Functors/index.md).  The corresponding heat transfer is:

!equation
q \equiv \mathbf{q}\cdot\mathbf{n}_\text{solid} = h (T_\text{solid} - T_\text{fluid}) \,,

The heat transfer coefficient $h$ is computed using the non-dimensional Churchill-Chu correlation [!cite](churchill1975):

!equation
\text{Nu}_D = \left[0.6 + \frac{0.387 \text{Ra}_D^{1/6}}{\left[1 + \left(\frac{0.559}{\text{Pr}}\right)^{9/16}\right]^{8/27}}\right]^2

The non-dimensional parameters are:

- Nusselt

!equation
\text{Nu}_D = \frac{hD}{k}

- Prandtl, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/Pr)

!equation
\text{Pr} = \frac{\mu c_p}{k}

- Rayleigh (Grashof times Prandtl), where Grashof is  given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/Gr)

!equation
\text{Ra}_D = \text{Gr}_D \cdot \text{Pr} = \frac{g\beta(T_\text{solid} - T_\text{fluid})D^3\rho^2c_p}{\mu k}

where

- $h$ is the heat transfer coefficient, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/htc_name),
- $\mathbf{n}_\text{solid}$ is the outward normal unit vector from the solid surface,
- $T_\text{solid}$ is the solid temperature,
- $T_\text{fluid}$ is the fluid temperature,
- $D$ is the solid cylinder diameter, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/diameter)
- $k$ is the fluid thermal conductivity, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/k_fluid)
- $\mu$ is the fluid dynamic viscosity,
- $c_p$ is the fluid isobaric specific heat,
- $g$ is the gravitational acceleration, 9.807 m/s$^2$,
- $\beta$ is the fluid volumetric expansion coefficient,
- $\rho$ is the fluid density.

The correlation in generally valid for air with $10^{-7}\lt \text{Ra}_D\lt 10^{13}$.

The AD version of this class is used to retrieve all of the input functors with AD types.

!syntax parameters /FunctorMaterials/ChurchillChuHTCFunctorMaterial

!syntax inputs /FunctorMaterials/ChurchillChuHTCFunctorMaterial

!syntax children /FunctorMaterials/ChurchillChuHTCFunctorMaterial


