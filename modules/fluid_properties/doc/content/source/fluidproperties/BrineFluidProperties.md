# BrineFluidProperties

!syntax description /FluidProperties/BrineFluidProperties

A high-precision and consistent formulation for fluid properties for binary salt (NaCl) and water
mixtures at pressures and temperatures of interest.

Density, enthalpy, internal energy and specific heat capacity are
calculated using the formulations provided in [!cite](driesner2007a) and [!cite](driesner2007b).

Viscosity and thermal conductivity of brine are calculated using the formulation of [!cite](phillips1981).

Brine vapor pressure is calculated using the formulation presented in [!cite](haas1976).

Solubility of solid salt (halite) in water is given by [!cite](potter1977).

By default, the BrineFluidProperties UserObject uses the [Water97FluidProperties](/Water97FluidProperties.md)
and [NaClFluidProperties](/NaClFluidProperties.md) which are constructed internally, so do not have to be
supplied by the user.

The BrineFluidProperties UserObject takes an optional parameter `water_fp` which can be used to pass
a specific water formulation. This allows the user to use a tabulated version of the water properties
(using [TabulatedFluidProperties](/TabulatedFluidProperties.md)), which can significantly speed up the
calculation of brine properties. 

## Range of validity

The BrineFluidProperties UserObject is valid for:

- 273.15 K $\le$ T $\le$ 1273.15 K,
- 0.1 MPa $\le$ p $\le$ 500 MPa,
- 0 $\le$ x$_{\mathrm{nacl}}$ $\le$ 1

!syntax parameters /FluidProperties/BrineFluidProperties

!syntax inputs /FluidProperties/BrineFluidProperties

!syntax children /FluidProperties/BrineFluidProperties

!bibtex bibliography
