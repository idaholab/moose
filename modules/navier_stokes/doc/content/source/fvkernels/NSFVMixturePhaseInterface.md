# NSFVMixturePhaseInterface

This terms models the volumetric exchange between two phases.
The term added reads as follows:

\begin{equation}
  \alpha_{1,2} (\phi_1 - \phi_2) \,,
\end{equation}

where:

- $\alpha_{1,2}$ is the volumetric exchange coefficient between phases 1 and 2
- $\phi_1$ is the volume fraction of phase 1,
- $\phi_2$ is the volume fraction of phase 2

!alert note
The kernel adds to the residual for the phase identified in `variable`.
The complementary kernel for the second phase is needed to preserve the phase fraction.

!syntax parameters /FVKernels/NSFVMixturePhaseInterface

!syntax inputs /FVKernels/NSFVMixturePhaseInterface

!syntax children /FVKernels/NSFVMixturePhaseInterface
