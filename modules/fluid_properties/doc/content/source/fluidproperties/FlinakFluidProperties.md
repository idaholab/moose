# FlinakFluidProperties

!syntax description /FluidProperties/FlinakFluidProperties

## Description

The `FlinakFluidProperties` class provides fluid properties for
a molar eutectic composition of 46.5% LiF, 11.5% NaF, and 42% KF,
commonly referred to as 'flinak'.

Density is calculated from [!cite](richard), but with a pressure dependence
added to ensure finite derivatives with respect to pressure needed by some
applications. The partial derivative of density with respect to pressure is
assumed to be 1.7324e-7 kg/m$^3$/Pa [!cite](richard), but this may be set to
a user-defined value. Slightly increasing the partial derivative of density
with respect to pressure may improve convergence of compressible flow equations
without significantly affecting the physical accuracy of the density estimation
[!cite](scarlat). In the absense of the pressure dependence, the uncertainty
on density is $\pm$2% [!cite](richard).

Viscosity, isobaric specific heat, and thermal conductivity are calculated
with uncertainties of $\pm$20%, $\pm$20%, and $\pm$15%, respectively
[!cite](richard).

Isochoric specific heat is calculated according to its definition as

\begin{equation}
C_v=\left(\frac{\partial e}{\partial T}\right)_v
\end{equation}

which becomes, after substituting the definition for $e$,

\begin{equation}
C_v=\left(\frac{\partial h}{\partial T}\right)_v-\left(\frac{\partial(Pv)}{\partial T}\right)_v
\end{equation}

## Range of Validity

These fluid properties are only applicable to liquid flinak. At atmospheric
pressure, the melting and boiling points of flibe are approximately 454$\degree$C
and 1570$\degree$C, respectively [!cite](richard). These fluid properties should
not be used outside this range.

!syntax parameters /FluidProperties/FlinakFluidProperties

!syntax inputs /FluidProperties/FlinakFluidProperties

!syntax children /FluidProperties/FlinakFluidProperties
