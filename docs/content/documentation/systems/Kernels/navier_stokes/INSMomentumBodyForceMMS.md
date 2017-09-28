# INSMomentumBodyForceMMS

## Description

Used during method of manufactured solutions (MMS) testing to consistently
include the MMS forcing function into the streamline-upwind Petrov-Galerkin
(SUPG) formulation. This kernel should be applied to each velocity component
present in the Navier Stokes simulation. This kernel's finite element weak form
is given by:

\begin{equation}
R_i(u_h) = -\tau \vec{U} \cdot \nabla \psi_i f \forall \psi_i
\end{eqaution}

where $\tau$ is the SUPG stabilization parameter, $\vec{U}$ is the velocity,
$\psi_i$ is the test function, and $f$ is the MMS forcing function for the
velocity component the kernel is being applied to.

## Example Syntax

Example use of `INSMomentumBodyForceMMS` can be found in the kernel block below:

!listing modules/navier_stokes/test/tests/ins/mms/supg/supg_mms_test.i block=Kernels label=False

!syntax parameters /Kernels/INSMomentumBodyForceMMS

!syntax inputs /Kernels/INSMomentumBodyForceMMS

!syntax children /Kernels/INSMomentumBodyForceMMS
