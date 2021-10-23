# PNSFVMomentumPressureFluxRZ

!syntax description /FVKernels/PNSFVMomentumPressureFluxRZ

## Overview

This object adds a residual equivalent to

\begin{equation}
\int_{\Omega_C} -\epsilon \frac{p}{r} dV
\end{equation}

for use when performing axisymmetric simulations with `Problem/coord_type=RZ`
and when the $\epsilon \nabla p$ term has been integrated by parts as is done
for [PCNSFVKT.md] and for [PCNSFVHLLC.md].

!syntax parameters /FVKernels/PNSFVMomentumPressureFluxRZ

!syntax inputs /FVKernels/PNSFVMomentumPressureFluxRZ

!syntax children /FVKernels/PNSFVMomentumPressureFluxRZ
