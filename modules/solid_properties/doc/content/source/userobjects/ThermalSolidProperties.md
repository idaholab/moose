# ThermalSolidProperties

This is the base class for providing thermal solid properties
as a function of temperature.
This class defines functions to compute the following thermal properties
as a function of temperature:

- compute isobaric specific heat - `Real cp_from_T(const Real & T)`
- compute thermal conductivity - `Real k_from_T(const Real & T)`
- compute density - `Real rho_from_T(const Real & T)`

Functions to compute derivatives of these properties as a function of temperature
are also available:

- compute isobaric specific heat and its temperature derivative - `void cp_from_T(const Real & T, Real & cp, Real & dcp_dT)`
- compute thermal conductivity and its temperature derivative - `void k_from_T(const Real & T, Real & k, Real & dk_dT)`
- compute density and its temperature derivative - `void rho_from_T(const Real & T, Real & rho, Real & drho_dT)`

To create a new userobject providing thermal properties, derive from this
userobject and specify implementations of the above functions.

!bibtex bibliography
