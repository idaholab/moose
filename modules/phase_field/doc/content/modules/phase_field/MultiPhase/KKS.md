# Kim-Kim-Suzuki Model

The Kim-Kim-Suzuki (KKS) model [!citep](kim_phase-field_1999) introduces a
separate concentration for every independent chemical component in every
phase. This permits the interfacial energy to be selected independently of the
diffuse-interface width, which is particularly useful for systems with a large
heat of solution.

For a system with $P$ phases and $N$ independent concentrations, let $c_i$ be
the global concentration of component $i$, $c_{i,p}$ its concentration in
phase $p$, and $h_p$ the switching function for phase $p$. The KKS
concentration constraints are

\begin{equation}
c_i = \sum_{p=1}^{P} h_p c_{i,p}, \qquad i=1,\ldots,N.
\label{eq:kks-concentration-constraint}
\end{equation}

The phase concentrations also satisfy pointwise equality of the chemical
potentials,

\begin{equation}
\frac{\partial F_p}{\partial c_{i,p}} =
\frac{\partial F_q}{\partial c_{i,q}}, \qquad
i=1,\ldots,N, \quad p,q=1,\ldots,P.
\label{eq:kks-chemical-potential-constraint}
\end{equation}

Each phase free energy $F_p$ may depend on every independent concentration in
that phase. For a substitutional system with $C$ chemical components whose
mole fractions sum to one, only $N=C-1$ concentrations are independent.

The two-phase objects use one order parameter and the specializations

\begin{equation}
c_i = \left(1-h(\eta)\right)c_{i,a} + h(\eta)c_{i,b}
\end{equation}

and

\begin{equation}
F = \left(1-h(\eta)\right)F_a + h(\eta)F_b + Wg(\eta).
\end{equation}

The `KKSMulti` objects generalize these equations to multiple phases and
multiple order parameters. Both the global and nested solution approaches
described below support multiple independent concentrations.

## Global phase-concentration solve

In the global solve, every $c_{i,p}$ is a nonlinear variable. For each
independent concentration, the input contains:

- one [KKSMultiPhaseConcentration.md] kernel for
  [eq:kks-concentration-constraint];
- $P-1$ [KKSPhaseChemicalPotential.md] kernels that enforce
  [eq:kks-chemical-potential-constraint] between a connected sequence of
  phase pairs; and
- a split Cahn-Hilliard system containing [KKSSplitCHCRes.md],
  [CoupledTimeDerivative.md], and [SplitCHWRes.md].

For every phase order parameter, the Allen-Cahn equation contains one
[KKSMultiACBulkF.md] and one [KKSMultiACBulkC.md] for each independent
concentration. The phase free energies must include all phase concentrations
on which they depend so that the cross-component derivatives are available to
the kernels.

The two-phase [KKSPhaseConcentration.md], [KKSACBulkF.md], and
[KKSACBulkC.md] objects are conveniences for $P=2$; they should not be mixed
with the multiphase Allen-Cahn formulation.

## Nested phase-concentration solve

The nested solve keeps only the global concentrations and phase order
parameters in the global nonlinear system. At every quadrature point,
[KKSPhaseConcentrationMultiPhaseMaterial.md] solves the $NP$ KKS constraint
equations for the phase concentrations as material properties, and
[KKSPhaseConcentrationMultiPhaseDerivatives.md] supplies their derivatives to
the global Jacobian.

The phase concentrations must be listed with the phase index varying fastest.
For example, a three-phase model with two independent concentrations `c` and
`b` uses

```text
global_cs = 'c b'
all_etas = 'eta1 eta2 eta3'
ci_names = 'c1 c2 c3 b1 b2 b3'
Fj_names = 'F1 F2 F3'
hj_names = 'h1 h2 h3'
```

The corresponding global equations use [NestedKKSMultiACBulkF.md],
[NestedKKSMultiACBulkC.md], and [NestedKKSMultiSplitCHCRes.md]. The free-energy
materials evaluated by the nested solve must set `compute = false`, as shown
in the examples on [KKSPhaseConcentrationMultiPhaseMaterial.md].

The nested solve reduces the number of globally coupled nonlinear variables,
but its local system grows with the product $NP$. Models with many phases or
components should monitor nested convergence and the conditioning of the
constraint equations.

## More than three phases

The `KKSMulti` kernels accept lists of phase free energies, switching
functions, phase concentrations, and order parameters, and are not restricted
to three phases. The [SwitchingFunctionMultiPhaseMaterial.md] can provide
switching functions and their derivatives for an arbitrary number of order
parameters. The KKS multiphase concentration objects currently assume one
phase concentration and one phase order parameter per phase.

## Examples and verification status

- [KKSMultiComponentExample.md] presents a two-phase ternary model with two
  independent concentrations.
- The [three-phase global input](/kks_multiphase.i) uses one independent
  concentration.
- The [three-phase nested input](/kks_example_multiphase_nested.i) uses one
  independent concentration.
- The [three-phase ternary global example](/kks_example_3phase_ternary_global.i)
  uses two independent concentrations with circular initial phase regions.
- The [three-phase ternary nested example](/kks_example_3phase_ternary_nested.i)
  uses the corresponding nested solve.

The [KKSAction.md] generates either the global or nested formulation from a
common phase/component description. Both action regression tests exercise
three phases and two independent concentrations in the same system.

## See also

- [KKSDerivations.md]
- [KKSAnalytical.md]
- [SLKKS.md]
