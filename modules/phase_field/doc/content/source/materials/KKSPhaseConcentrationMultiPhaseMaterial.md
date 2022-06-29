# KKSPhaseConcentrationMultiPhaseMaterial

Kim-Kim-Suzuki (KKS) nested solve material for multiphase models (part 1 of 2). KKSPhaseConcentrationMultiPhaseMaterial implements a nested Newton iteration to solve the KKS constraint equations for the phase concentrations $c_{i,p}$ as material properties (instead of non-linear variables as in the traditional solve in MOOSE), where $i$ is the component species and $p$ is the phase. For a model with N phases, the constraint equations are the mass conservation equation for each global concentration ($c_i$):

\begin{equation}
c_i=\sum_{p=1}^N h(\eta_p)c_{i,p},
\end{equation}

and the pointwise equality of the phase chemical potentials:

\begin{equation}
\frac{\partial f_a}{\partial c_{i,a}} = \frac{\partial f_b}{\partial c_{i,b}}.
\end{equation}

The free energies in Fj_materials must have *compute=false*. This material also passes the phase free energies and their partial derivatives w.r.t phase concentrations to the KKS kernels (NestKKSMultiACBulkC, NestKKSMultiACBulkF, NestKKSSplitCHCRes).

## Example input:

!listing kks_example_multiphase_nested.i block=Materials

## Class Description

!syntax description /Materials/KKSPhaseConcentrationMultiPhaseMaterial

!syntax parameters /Materials/KKSPhaseConcentrationMultiPhaseMaterial

!syntax inputs /Materials/KKSPhaseConcentrationMultiPhaseMaterial

!syntax children /Materials/KKSPhaseConcentrationMultiPhaseMaterial
