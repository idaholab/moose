# INSADSmagorinskyEddyViscosity

This object computes the residual and Jacobian contribution of the eddy viscosity term
as given by the Smagorinsky large eddy simulation model. For some details on how this
tends to be used, the paper [!cite](bouffanais2007) is recommended. Using this kernel
adds this term to the momentum equation:

\begin{equation}
-\nu_{sgs} \rho \nabla^2 \vec u
\end{equation}

Where the subgrid-scale eddy viscosity is calculated as:

\begin{equation}
\nu_{sgs} = (C_s \bar{\Delta})^2 ||\bar{\bar{S}}||
\end{equation}

Where the symmetric strain rate tensor magnitude is calculated by:
\begin{equation}
||\bar{\bar{S}}||^2 = \sum_{i=0}^3 \sum_{j=0}^3 (\nabla \vec u + \nabla \vec u^T)^2
\end{equation}

The filter length can be calculated in a few ways, and this kernel uses the common approach
of using the cube root of the element volume as the filter length, divided by the local
element polynomial degree for the velocity variable. The Smagorinsky constant $C_s$ comes
from theory [!cite](smagorinsky1963), and is set by default to a reasonable value.

!listing modules/navier_stokes/test/tests/finite_element/ins/lid_driven/ad_lid_driven_les.i block=Kernels

!syntax description /Kernels/INSADSmagorinskyEddyViscosity

!syntax parameters /Kernels/INSADSmagorinskyEddyViscosity

!syntax inputs /Kernels/INSADSmagorinskyEddyViscosity

!syntax children /Kernels/INSADSmagorinskyEddyViscosity

!bibtex bibliography
