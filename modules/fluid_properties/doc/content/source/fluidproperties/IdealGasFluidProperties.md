# IdealGasFluidProperties

These fluid properties implement the ideal gas law:

!equation
p v = \frac{R T}{M} \,,

where

- $p$ is pressure,
- $v$ is specific volume,
- $R$ is the universal gas constant,
- $T$ is the temperature (in absolute units), and
- $M$ is the molar mass.

The specific heats (isobaric, $c_p$, and isochoric, $c_v$) are assumed constant,
and thus their ratio is constant as well:

!equation
\gamma = \frac{c_p}{c_v} \,.

The specific internal energy is computed as

!equation
e = e_\text{ref} + c_v T \,,

where $e_\text{ref}$ is a reference specific internal energy value (corresponding to $T = 0$).

The dynamic viscosity $\mu$ and thermal conductivity $k$ are assumed constant,
though this assumption could later be dropped.

!syntax parameters /FluidProperties/IdealGasFluidProperties

!syntax inputs /FluidProperties/IdealGasFluidProperties

!syntax children /FluidProperties/IdealGasFluidProperties
