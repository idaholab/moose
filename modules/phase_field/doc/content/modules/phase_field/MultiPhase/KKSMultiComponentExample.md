# Kim-Kim-Suzuki Example for Three or More Components

!listing modules/phase_field/examples/kim-kim-suzuki/kks_example_ternary.i id=kks-ternary-example caption=Two-phase ternary KKS input.

This example is a two-phase ternary system. It tracks two independent
concentrations because the three component mole fractions sum to one. More
generally, a substitutional system with $C$ chemical components requires
$C-1$ independent Cahn-Hilliard equations. Each Cahn-Hilliard equation in the
global phase-concentration solve requires the kernels:

- [KKSSplitCHCRes.md]
- [CoupledTimeDerivative.md]
- [SplitCHWRes.md]

To enforce the concentration and chemical-potential constraints, each
independent concentration also requires:

- one [KKSPhaseConcentration.md]; and
- one [KKSPhaseChemicalPotential.md].

The Allen-Cahn equation is also modified when additional components are added. The residual becomes

\begin{equation}
R=-\frac{dh}{d\eta} \left(F_a-F_b- \sum_{i=1}^{N} \frac{dF_a}{dc_{ia}}(c_{ia}-c_{ib})\right) + w\frac{dg}{d\eta}.
\end{equation}

where $N=C-1$ is the number of independent concentrations. A single
[KKSACBulkF.md] kernel is needed as in the binary case, and one
[KKSACBulkC.md] kernel is added for each independent concentration.

## Multiphase generalization

For $P>2$, replace the two-phase concentration and Allen-Cahn objects with
[KKSMultiPhaseConcentration.md], [KKSMultiACBulkF.md], and
[KKSMultiACBulkC.md]. For every independent concentration, use one
`KKSMultiPhaseConcentration`, $P-1$ `KKSPhaseChemicalPotential` kernels that
connect all phases, and one `KKSMultiACBulkC` in every order-parameter
equation. The complete object layout is described on [KKS.md].

The nested alternative uses [KKSPhaseConcentrationMultiPhaseMaterial.md] and
[KKSPhaseConcentrationMultiPhaseDerivatives.md] to solve all phase
concentrations locally, together with the `NestedKKSMulti` kernels for the
global equations.

!alert note title=Example scope
The input listed above demonstrates multiple components in a two-phase model.
Three-phase ternary examples for both phase-concentration solution approaches
are provided with [KKSAction.md].
