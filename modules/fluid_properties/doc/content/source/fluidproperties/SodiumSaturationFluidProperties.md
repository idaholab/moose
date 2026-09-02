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

The speed of sound used for compressible-flow wave speeds is the Fink and Leibowitz correlation
fit to sodium measurements over the range $370.98 < T (K) < 1173$:

$c=2660.7-0.37667T-9.0356\times10^{-5}T^2$

Here, $c$ is in m/s. This independent correlation is used because the saturated-liquid density
fit neglects pressure dependence and therefore cannot supply a physical sound speed through its
pressure derivative.

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

The baseline isobaric and isochoric specific heats are obtained as fits to the
saturated-liquid experimental data in Fink and Leibowitz over the range
$400 < T (K) < 2200$:

$C_{p,0}(T)=3.7782\times10^{-10}T^4-1.7191\times10^{-6}T^3+3.0921\times10^{-3}T^2-2.4560T+1.972\times10^3$

$C_v(T)=1.0369\times10^{-8}T^3+3.7164\times10^{-4}T^2-1.0494T+1.5826\times10^3$

The $R^2$ values for both fits are 0.997. The $C_{p,0}$ fit matches the experimental
data to within 0.5%, while the $C_v$ fit matches the data to within 1.5%.
For the pressure extension described below, the implementation assigns the
$C_{p,0}(T)$ curve to the reference pressure $p_0=10^5$ Pa. Thus, the subscript
0 identifies the model baseline; it does not indicate that the original data
were measured along a separate constant-pressure isobar. The $C_v(T)$ fit is
used directly without a pressure correction.

At the reference pressure, the baseline enthalpy is defined by

$C_{p,0}(T)\equiv\frac{dh_0}{dT}$

and is therefore computed by integrating the empirical $C_{p,0}$ fit:

$h_0(T)-h_0(T_{ref})=\int_{T_{ref}}^T C_{p,0}(T')\,dT'.$

In the implementation this integral is written as

$h_0(T)=F(T)-401088.7\ {\rm J/kg},$

where $F(T)$ is an antiderivative of $C_{p,0}(T)$. The integration constant is
selected to match the Fink and Leibowitz enthalpy correlation at 371 K.
This construction enforces $dh_0/dT=C_{p,0}$ exactly and agrees with the
Fink and Leibowitz enthalpy correlation to within 0.2% over the valid range of
the $C_{p,0}$ fit.

A pressure extension is then derived from this baseline enthalpy and the
temperature-only specific-volume correlation $v(T)=1/\rho(T)$. For a simple
compressible system, the Gibbs free-energy and enthalpy differentials are

$dg=-s\,dT+v\,dp$

and

$dh=T\,ds+v\,dp$ [!cite](callen1985).

Equality of the mixed partial derivatives of $g$ gives the Maxwell relation

$\left(\frac{\partial s}{\partial p}\right)_T=-\left(\frac{\partial v}{\partial T}\right)_p=-\frac{dv}{dT}$.

At $p_0$, the entropy baseline follows from
$ds_0=C_{p,0}(T)\,dT/T$:

$s_0(T)-s_0(T_{ref})=\int_{T_{ref}}^T\frac{C_{p,0}(T')}{T'}\,dT'.$

The entropy datum is chosen as $s_0(370.98\ {\rm K})=0$. Because $v$ depends
only on temperature in this model, integration of the Maxwell relation with
respect to pressure from $p_0$ then gives

$s(p,T)=s_0(T)-(p-p_0)\frac{dv}{dT}$.

At constant temperature, the enthalpy form of the Gibbs relation then gives

$\left(\frac{\partial h}{\partial p}\right)_T=T\left(\frac{\partial s}{\partial p}\right)_T+v=v-T\frac{dv}{dT}$,

which integrates to

$h(p,T)=h_0(T)+(p-p_0)\left[v(T)-T\frac{dv}{dT}\right]$.

Finally, applying the definition
$C_p=(\partial h/\partial T)_p$ to this pressure-extended enthalpy gives

$C_p(p,T)=C_{p,0}(T)-(p-p_0)T\frac{d^2v}{dT^2}.$

Therefore, the polynomial above supplies the empirical baseline, while the
second term is not another empirical fit: it follows from differentiating the
Gibbs-consistent pressure extension. At $p=p_0$, this correction vanishes and
$C_p(p_0,T)=C_{p,0}(T)$.

These expressions satisfy the Gibbs relation exactly within the assumed
$v=v(T)$ model. They are only a first-order pressure extension of the original
saturated-liquid correlations because pressure dependence of $v$ and other
higher-order pressure effects are neglected; they do not constitute a full
subcooled-sodium equation of state. In particular, the independent $C_v(T)$
correlation is retained rather than derived from this pressure extension.
The inverse temperature relation $T(p,h)$ is obtained by Newton inversion of
this pressure-extended enthalpy correlation.

## Range of Validity

The underlying empirical correlations describe saturated liquid sodium. Values
away from $p_0$ use the first-order pressure extension described above and
should not be interpreted as a validated subcooled-sodium equation of state.

!syntax parameters /FluidProperties/SodiumSaturationFluidProperties

!syntax inputs /FluidProperties/SodiumSaturationFluidProperties

!syntax children /FluidProperties/SodiumSaturationFluidProperties
