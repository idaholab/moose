# (AD)ChurchillChuHTCFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes a natural convection heat
flux $q$ between a long, horizontal, cylindrical, heated solid surface and a fluid using [functors](/Functors/index.md):

!equation
q \equiv \mathbf{q}\cdot\mathbf{n}_\text{solid} = h (T_\text{solid} - T_\text{fluid}) \,,

The heat transfer coefficient $h$ is computed using the non-dimensional Churchill-Chu correlation [!cite](churchill1975):

!equation
Nu_D = \left[0.6 + \frac{0.387 Ra_D^{1/6}}{\left[1 + \left(\frac{0.559}{Pr}\right)^{9/16}\right]^{8/27}}\right]^2

The non-dimensional parameters are:

- Nusselt

!equation
Nu_D = \frac{hD}{k}

- Prandtl

!equation
Pr = \frac{\mu c_p}{k}

- Rayleigh (Grashof times Prandtl)

!equation
Ra_D = Gr_D \cdot Pr = \frac{g\beta(T_\text{solid} - T_\text{fluid})D^3\rho^2c_p}{\mu k}

where

- $h$ is the heat transfer coefficient, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/htc),
- $\mathbf{n}_\text{solid}$ is the outward normal unit vector from the solid surface,
- $T_\text{solid}$ is the solid temperature, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/T_solid), and
- $T_\text{fluid}$ is the fluid temperature, given by [!param](/FunctorMaterials/ChurchillChuHTCFunctorMaterial/T_fluid)
- $D$ is the solid cylinder diameter,
- $k$ is the fluid thermal conductivity,
- $\mu$ is the fluid dynamic viscosity,
- $c_p$ is the fluid isobaric specific heat,
- $g$ is the gravitational acceleration, $9.807 \frac{\text{m}}{\text{s}^2}$,
- $\beta$ is the fluid volumetric expansion coefficient,
- $\rho$ is the fluid density.

The fluid material is set by...

All fluid properties are evaluted at the film temperature, the arithmetic averge of $T_\text{solid}$ and $T_\text{fluid}$.

The correlation in generally valid for air with $10^{-7}\lt Ra_D\lt 10^{13}$.

The AD version of this class is used to retrieve all of the input functors with AD types.

!syntax parameters /FunctorMaterials/ChurchillChuHTCFunctorMaterial

!syntax inputs /FunctorMaterials/ChurchillChuHTCFunctorMaterial

!syntax children /FunctorMaterials/ChurchillChuHTCFunctorMaterial


