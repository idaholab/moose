# SodiumSaturationFluidProperties

!syntax description /FluidProperties/SodiumSaturationFluidProperties

## Description

The `SodiumSaturationFluidProperties` class provides fluid properties for _saturated_ liquid
sodium based on correlations used in the SAS4A/SASSYS-1 reactor dynamics
and safety analysis code developed at Argonne National Laboratory for
liquid metal reactors [!cite](sas). These property models are obtained as
fits to experimental data, with computational efficiency
motivating the use of a simpler functional fits than proposed in the original
references upon which the SAS4a/SASSYS-1 implementation is based,
namely [!cite](fink). Only for $C_p$ and $C_v$ are the original correlations/data
used, since the SAS4A/SASSYS-1 implementation does not differentiate between
$C_p$ and $C_v$ for the saturated liquid.

Density is calculated as an empirical fit to two saturated liquid
density correlations recommended by Fink and Leibowitz that
cover the range $371 < T (K) < 2509$:

$\rho=1.00423\times10^3-0.21390T-1.1046\times10^{-5}T^2$

This equation fits the Fink and Leibowitz models to within 9.5%.

The thermal conductivity is a fit to experimental data by Fink and Leibowitz
below 1500 K, and extrapolated values above 1500 K based on
a method described by Grosse [!cite](sas):

$k=1.1045\times10^2-6.5112\times10^{-2}T+1.5430\times10^{-5}T^2-2.4617\times10^{-9}T^3$

This equation fits the Fink and Leibowitz data to within 0.5%.

The dynamic viscosity is given as a fit to experimental data by Fink and
Leibowitz below 1200 K and extrapolated values about 1200 K based on
a method described by Grosse [!cite](sas):

$\mu=3.6522\times10^{-5}+\frac{0.16626}{T}-\frac{4.56877\times10^1}{T^2}+\frac{2.8733\times10^4}{T^3}$

This equation fits the Fink and Leibowitz data to within 0.5%.

The isobaric and isochoric specific heats are obtained as new fits to the
experimental data in Fink and Leibowitz over the range $400 < T (K) < 2200$,

$C_p=3.7782\times10^{-1}T^2-1.7191\times10^{-6}T^3+3.0921\times10^{-3}T^2-2.4560T+1.972\times10^3$

$C_v=1.0369\times10^{-8}T^3+3.7164\times10^{-4}T^2-1.0494T+1.5826\times10^3$

The $R^2$ values for both fits are 0.997, where the $C_p$ fit matches experimental
data to within 0.5%, while for $C_v$ matches experimental data to within 1.5%.

Enthalpy is computed based on the definition of $C_p$,

$C_p\equiv\left(\frac{\partial h}{\partial T}\right)_p$

Neglecting any variation in the saturation conditions with respect to pressure
(i.e. Poynting-type effects), $h$ can be computed by integrating the above
expression,

$h(T)-h(T_{ref})=\int_{T_{ref}}^TC_pdT'$

where $T_{ref}$ is a reference temperature and $h(T_{ref})$ is the enthalpy
at that reference temperature. This expression is used to evaluate enthalpy
based on the $C_p$ fit as

$h(T)=F(T)-401088.7$

where $F(T)$ is the integral of $C_p$ with respect to temperature evaluated
at $T$ and $401088.7$ J/kg/K is a constant representing $-F(T_{ref})+h(T_{ref})$
selected in order to match the Fink and Leibowitz correlation at 371 K.
This approach, rather than simply using the Fink and Leibowitz correlation outright,
is used to ensure exact compatibility with thermodynamic definitions and the paricular
fit for $C_p$ selected in this class.
Computing $h$ based on an integral of $C_p$ matches the Fink
and Leibowitz correlations to within 0.2% over the entire range for which
the $C_p$ fit is valid.

## Range of Validity

These fluid properties correspond to saturated conditions (that is, the sodium is at the
saturation pressure).

!syntax parameters /FluidProperties/SodiumSaturationFluidProperties

!syntax inputs /FluidProperties/SodiumSaturationFluidProperties

!syntax children /FluidProperties/SodiumSaturationFluidProperties
