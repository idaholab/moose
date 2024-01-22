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
The kernel simultaneously adds to the residual of phases 1 and 2 if the transport
of both phases are included in the model. So, there is no need to add a separate
kernel for each phase.

!syntax parameters /FVKernels/NSFVMixturePhaseInterface

!syntax inputs /FVKernels/NSFVMixturePhaseInterface

!syntax children /FVKernels/NSFVMixturePhaseInterface
