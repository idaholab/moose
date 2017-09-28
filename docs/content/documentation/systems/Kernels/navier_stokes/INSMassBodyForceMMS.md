# INSMassBodyForceMMS

## Description

Used during method of manufactured solutions (MMS) testing to consistently include
the MMS forcing functions into the pressure-stabilized Petrov-Galerkin (PSPG)
formulation. This kernel is applied to the pressure variable. This kernel's
finite element weak form is given by:

\begin{equation}
R_i(u_h) = \tau \nabla \psi_i \cdot \vec{f} \forall \psi_i
\end{eqaution}

where $\tau$ is the PSPG stabilization parameter,
$\psi_i$ is the test function, and $\vec{f}$ is a vector whose components
correspond to the MMS forcing functions of the respective velocity components.

## Example Syntax

Example use of `INSMassBodyForceMMS` can be found in the kernel block below:

!listing modules/navier_stokes/test/tests/ins/mms/pspg/pspg_mms_test.i block=Kernels label=False

!syntax parameters /Kernels/INSMassBodyForceMMS

!syntax inputs /Kernels/INSMassBodyForceMMS

!syntax children /Kernels/INSMassBodyForceMMS
