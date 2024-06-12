# KKSPhaseConcentrationMaterial

Kim-Kim-Suzuki (KKS) nested solve material (part 1 of 2). KKSPhaseConcentrationMaterial implements a nested Newton iteration to solve the KKS constraint equations for the phase concentrations $c_a$ and $c_b$ as material properties (instead of non-linear variables as in the traditional solve in MOOSE). The constraint equations are the mass conservation equation for each global concentration ($c$):

\begin{equation}
c=\left(1-h(\eta)\right)c_a + h(\eta)c_b,
\end{equation}

and the pointwise equality of the phase chemical potentials:

\begin{equation}
\frac{\partial f_a}{\partial c_a} = \frac{\partial f_b}{\partial c_b}.
\end{equation}

The parameters Fa_material and Fb_material must have [!param](/Materials/KKSPhaseConcentrationMaterial/compute) set to `false`. This material also passes the phase free energies and their partial derivatives w.r.t phase concentrations to the KKS kernels (NestKKSACBulkC, NestKKSACBulkF, NestKKSSplitCHCRes).

## Example input:

### Without damping

Parabolic free energies have valid values for any real number, and therefore don't require damping to ensure the solution is inside a trust region.

!listing kks_example_nested.i block=Materials

### With damping

Log free energies are only valid when the component phase mole fractions are within 0 to 1. We add a material `C` that checks if the nested solve guess is within this trust region. Similar to the free energy, `C` must have [!param](/Materials/KKSPhaseConcentrationMaterial/compute) set to `false`. The nested solve then requires damping to ensure the solution is inside the trust region.

!listing kks_example_nested_damped.i block=Materials

## Class Description

!syntax description /Materials/KKSPhaseConcentrationMaterial

!syntax parameters /Materials/KKSPhaseConcentrationMaterial

!syntax inputs /Materials/KKSPhaseConcentrationMaterial

!syntax children /Materials/KKSPhaseConcentrationMaterial
