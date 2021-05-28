# PCNSFVMomentumPressureRZ

!syntax description /FVKernels/PCNSFVMomentumPressureRZ

## Overview

This object adds a residual equivalent to

\begin{equation}
\int_{\Omega_C} -\epsilon \frac{p}{r} dV
\end{equation}

for use when performing axisymmetric simulations with `Problem/coord_type=RZ`
and when the $\epsilon \nabla p$ term has been integrated by parts as is done
for [PCNSFVKT.md] and for [PCNSFVHLLC.md].

!syntax parameters /FVKernels/PCNSFVMomentumPressureRZ

!syntax inputs /FVKernels/PCNSFVMomentumPressureRZ

!syntax children /FVKernels/PCNSFVMomentumPressureRZ
