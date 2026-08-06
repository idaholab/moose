# KKSAction

!syntax description /Modules/PhaseField/KKS/KKSAction

## Description

`KKSAction` sets up a multiphase, multicomponent Kim-Kim-Suzuki model using
one order parameter per phase and the split Cahn-Hilliard formulation. The
phase free energies, switching functions, barrier functions, mobilities, and
gradient energy coefficients are supplied by the user. The action creates the
nonlinear variables and kernels that couple those properties.

For $P$ phases and $N$ independent concentrations, the generated equations
enforce the concentration constraints

\begin{equation}
c_i = \sum_{p=1}^{P} h_p c_{i,p}, \qquad i=1,\ldots,N,
\end{equation}

and equality of the chemical potentials between the phases,

\begin{equation}
\frac{\partial F_p}{\partial c_{i,p}} =
\frac{\partial F_q}{\partial c_{i,q}}, \qquad
i=1,\ldots,N, \quad p,q=1,\ldots,P.
\end{equation}

See [KKS.md] for the complete model description.

For each name in
[!param](/Modules/PhaseField/KKS/KKSAction/global_concentrations), the action
creates a global concentration and a chemical-potential variable named
`mu_<concentration>`. For example, `global_concentrations = 'c b'` creates
`c`, `mu_c`, `b`, and `mu_b`. It also creates every variable listed in
[!param](/Modules/PhaseField/KKS/KKSAction/order_parameters).

The phase concentration names combine the global concentration and phase
names supplied in [!param](/Modules/PhaseField/KKS/KKSAction/phase_names).
For `phase_names = 'alpha beta gamma'` and
`global_concentrations = 'c b'`, the phase concentration order is
`c_alpha c_beta c_gamma b_alpha b_beta b_gamma`. This component-major order,
with the phase index varying fastest, is used throughout the generated model.

## Phase Concentration Solves

With
[!param](/Modules/PhaseField/KKS/KKSAction/phase_concentration_solve) set to
`GLOBAL`, every phase concentration is a nonlinear variable. For each
independent concentration, the action adds one
[KKSMultiPhaseConcentration.md], a connected sequence of $P-1$
[KKSPhaseChemicalPotential.md] kernels, and the split Cahn-Hilliard kernels.
Nonzero initial values for the generated phase concentration variables can be
supplied in the `[ICs]` block.

With `phase_concentration_solve = NESTED`, the phase concentrations are
material properties. The action creates
[KKSPhaseConcentrationMultiPhaseMaterial.md] and
[KKSPhaseConcentrationMultiPhaseDerivatives.md], then uses the
`NestedKKSMulti` kernels for the global equations. The parameter
[!param](/Modules/PhaseField/KKS/KKSAction/phase_concentration_initial_values)
is required and must use the component-major ordering described above. Its
entries may be literal values or auxiliary variables whose initial conditions
provide spatially varying values. Each phase free-energy material must have
the same object and property name supplied in
[!param](/Modules/PhaseField/KKS/KKSAction/free_energies), and must set
`compute = false` so the nested material can evaluate it.

The action exposes the standard nested Newton tolerances and iteration limits.
It also supports damped nested solves through
[!param](/Modules/PhaseField/KKS/KKSAction/damped_Newton) and
[!param](/Modules/PhaseField/KKS/KKSAction/conditions).

## Phase Constraint

The default
[!param](/Modules/PhaseField/KKS/KKSAction/phase_constraint) value,
`LAGRANGE`, creates the variable named by
[!param](/Modules/PhaseField/KKS/KKSAction/lagrange_multiplier), one
[SwitchingFunctionConstraintLagrange.md], and one
[SwitchingFunctionConstraintEta.md] per phase. Use `phase_constraint = NONE`
when the supplied switching functions are normalized by construction, such as
[SwitchingFunctionMultiPhaseMaterial.md].

## Example Input Syntax

The following block configures a three-phase ternary system with globally
solved phase concentrations:

!listing modules/phase_field/test/tests/actions/kks_3phase_ternary_global.i block=Modules id=kks-action-global caption=Global phase-concentration solve using `KKSAction`.

The same phase and component layout can use the nested solve without adding
the phase concentrations to the global nonlinear system:

!listing modules/phase_field/test/tests/actions/kks_3phase_ternary_nested.i block=Modules id=kks-action-nested caption=Nested phase-concentration solve using `KKSAction`.

!alert note title=Current scope
`KKSAction` assumes one order parameter per phase and generates the split
Cahn-Hilliard formulation. It does not create thermodynamic or interpolation
materials, initial conditions, boundary conditions, executioner settings, or
output objects.

!syntax parameters /Modules/PhaseField/KKS/KKSAction

!syntax inputs /Modules/PhaseField/KKS/KKSAction

!syntax children /Modules/PhaseField/KKS/KKSAction
