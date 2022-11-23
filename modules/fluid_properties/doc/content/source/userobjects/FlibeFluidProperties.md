# FlibeFluidProperties

!syntax description /FluidProperties/FlibeFluidProperties

## Description

The `FlibeFluidProperties` class provides fluid properties for
a peritectic molar composition of 67% LiF and 33% BeF$_2$, commonly
referred to as 'flibe'.

Density is calculated from [!cite](richard), but with a pressure dependence
added to ensure finite derivatives with respect to pressure needed by some
applications. The partial derivative of density with respect to pressure is
assumed to be 1.7324e-7 kg/m$^3$/Pa [!cite](richard), but this may be set to
a user-defined value. Slightly increasing the partial derivative of density
with respect to pressure may improve convergence of compressible flow equations
without significantly affecting the physical accuracy of the density estimation
[!cite](scarlat). In the absense of the pressure dependence, the uncertainty
on density is $\pm$0.05% [!cite](richard).

Viscosity, isobaric specific heat, and thermal conductivity are calculated
with uncertainties of $\pm$20%, $\pm$2%, and $\pm$15%, respectively
[!cite](richard). The viscosity of LiF and BeF$_2$ vary by eight orders of
magnitude, so caution should be used if applying these fluid properties to
LiF-BeF$_2$ mixtures with slightly different ratios [!cite](romatoski).

Isochoric specific heat is calculated according to its definition as

\begin{equation}
C_v=\left(\frac{\partial e}{\partial T}\right)_v
\end{equation}

which becomes, after substituting the definition for $e$,

\begin{equation}
C_v=\left(\frac{\partial h}{\partial T}\right)_v-\left(\frac{\partial(Pv)}{\partial T}\right)_v
\end{equation}

Molar mass is calculated assuming 99.995% enrichment of lithium in the
Li-7 isotope.

## Range of Validity

These fluid properties are only applicable to liquid flibe. At atmospheric
pressure, the melting and boiling points of flibe are approximately 458$\degree$C
and 1400$\degree$C, respectively [!cite](richard). These fluid properties should
not be used outside this range.

!syntax parameters /FluidProperties/FlibeFluidProperties

!syntax inputs /FluidProperties/FlibeFluidProperties

!syntax children /FluidProperties/FlibeFluidProperties
