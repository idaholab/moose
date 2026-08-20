# KKSPhaseConcentrationMultiPhaseMaterial

!syntax description /Materials/KKSPhaseConcentrationMultiPhaseMaterial

## Description

`KKSPhaseConcentrationMultiPhaseMaterial` uses a nested Newton solve to
compute the phase concentrations $c_{i,p}$ as material properties instead of
nonlinear variables. Here, $i$ identifies an independent concentration and
$p$ identifies a phase. For a model with $P$ phases, the local constraint
system contains one concentration conservation equation for each global
concentration $c_i$,

\begin{equation}
c_i = \sum_{p=1}^{P} h_p c_{i,p},
\end{equation}

and pointwise equality of the phase chemical potentials,

\begin{equation}
\frac{\partial F_p}{\partial c_{i,p}} =
\frac{\partial F_q}{\partial c_{i,q}}, \qquad p,q=1,\ldots,P.
\end{equation}

The free-energy materials named in
[!param](/Materials/KKSPhaseConcentrationMultiPhaseMaterial/Fj_names) must set
[!param](/Materials/DerivativeParsedMaterial/compute) to `false`. This material
also makes the phase free energies and their phase-concentration derivatives
available to [NestedKKSMultiACBulkC.md], [NestedKKSMultiACBulkF.md], and
[NestedKKSMultiSplitCHCRes.md]. The associated
[KKSPhaseConcentrationMultiPhaseDerivatives.md] material supplies derivatives
of the nested solution to the global Jacobian.

The number of phase concentrations must equal the number of entries in
[!param](/Materials/KKSPhaseConcentrationMultiPhaseMaterial/global_cs) times
the number of entries in
[!param](/Materials/KKSPhaseConcentrationMultiPhaseMaterial/all_etas). List
the phase concentrations with the phase index varying fastest. For example, a
three-phase system with independent concentrations `c` and `b` uses
`ci_names = 'c1 c2 c3 b1 b2 b3'`. The initial values or auxiliary variables in
[!param](/Materials/KKSPhaseConcentrationMultiPhaseMaterial/ci_IC) must use the
same ordering. Literal values initialize the corresponding phase
concentrations uniformly, while auxiliary variables can provide spatially
varying initial values.

For every global concentration, the local solve contains one concentration
conservation equation and $P-1$ chemical-potential equality equations, where
$P$ is the number of phases. Consequently, this material supports multiple
phases and multiple independent concentrations in the same solve.

## Example Input Syntax

### Without damping

Parabolic free energies are valid for any real concentration and therefore do
not require damping to keep the local solution inside a trust region.

!listing modules/phase_field/test/tests/KKS_system/kks_example_multiphase_nested.i block=Materials id=kks-nested-material caption=Nested phase-concentration solve without damping.

### With damping

Logarithmic free energies are only valid when the phase mole fractions are
between zero and one. A material named `C` checks whether the nested iterate
is inside this trust region. Like the free-energy materials, `C` must have
[!param](/Materials/DerivativeParsedMaterial/compute) set to `false`. Enable
damping to keep the local solution inside the trust region.

!listing modules/phase_field/test/tests/KKS_system/kks_example_multiphase_nested_damped.i block=Materials id=kks-nested-material-damped caption=Nested phase-concentration solve with damping.

!syntax parameters /Materials/KKSPhaseConcentrationMultiPhaseMaterial

!syntax inputs /Materials/KKSPhaseConcentrationMultiPhaseMaterial

!syntax children /Materials/KKSPhaseConcentrationMultiPhaseMaterial
