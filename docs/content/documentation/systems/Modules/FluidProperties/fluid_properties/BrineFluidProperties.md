# BrineFluidProperties
!syntax description /Modules/FluidProperties/BrineFluidProperties


A high-precision and consistent formulation for fluid properties for binary salt (NaCl) and water
mixtures at pressures and temperatures of interest.

Density, enthalpy, internal energy and specific heat capacity are
calculated using the formulations provided in \citet{driesner2007a} and \citet{driesner2007b}.

Viscosity and thermal conductivity of brine are calculated using the formulation of \citet{phillips1981}.

Brine vapor pressure is calculated using the formulation presented in \citet{haas1976}.

Solubility of solid salt (halite) in water is given by \citet{potter1977}.

##Range of validity
The BrineFluidProperties UserObject is valid for:

- 273.15 K $\le$ T $\le$ 1273.15 K,
- 0.1 MPa $\le$ p $\le$ 500 MPa,
- 0 $\le$ x$_{\mathrm{nacl}}$ $\le$ 1

!syntax parameters /Modules/FluidProperties/BrineFluidProperties

!syntax inputs /Modules/FluidProperties/BrineFluidProperties

!syntax children /Modules/FluidProperties/BrineFluidProperties

## References
\bibliographystyle{unsrt}
\bibliography{fluid_properties.bib}
