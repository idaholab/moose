# ComputeFrictionalForceLMMechanicalContact

!syntax description /Constraints/ComputeFrictionalForceLMMechanicalContact

This constraint enforces quasistatic Coulomb friction with dual mortar Lagrange multipliers. The
normal multiplier \(p_n\) and tangential multiplier vector \(\boldsymbol p_t\) are physical
pressures; changing a complementarity formulation or scale strategy does not change their value
semantics. The implementation follows the nonsmooth augmented-multiplier formulations of
[!citep](alart1991mixed), [!citep](hueber2008primal), and [!citep](gitterle2010finite).

## Sign convention and projection maps

Let \(\bar g=G/M\) be the normalized mortar gap, positive in separation, and let
\(\bar{\boldsymbol d}\) be the normalized relative tangential displacement over the current time
step. For positive pressure scales \(C_n\) and \(C_t\), define

\begin{equation}
a=p_n-C_n\bar g, \qquad
R=\mu\max(0,a), \qquad
\boldsymbol q_t=\boldsymbol p_t+C_t\bar{\boldsymbol d}.
\end{equation}

The minus sign in \(a\) is required by the positive-separation gap convention: opening reduces the
augmented normal pressure and makes the Coulomb radius vanish. Two equivalent friction maps are
available through [!param](/Contact/ContactAction/friction_ncp_formulation):

\begin{equation}
\boldsymbol F_{\mathrm{AC}}=
\boldsymbol p_t-\operatorname{Proj}_{B_R}(\boldsymbol q_t),
\end{equation}

\begin{equation}
\boldsymbol F_{\mathrm{HSW}}=
\max(R,\lVert\boldsymbol q_t\rVert)\boldsymbol p_t-R\boldsymbol q_t.
\end{equation}

Here \(B_R\) is the closed tangential ball of radius \(R\). The Alart--Curnier map is homogeneous
of degree one in pressure. The Hueber--Stadler--Wohlmuth map is degree two and, whenever
\(w=\max(R,\lVert\boldsymbol q_t\rVert)>0\), satisfies

\begin{equation}
\boldsymbol F_{\mathrm{HSW}}=w\boldsymbol F_{\mathrm{AC}}.
\end{equation}

They therefore have the same stick and slip roots for \(w>0\). At the separated state
\(R=\lVert\boldsymbol q_t\rVert=0\), the unguarded degree-two expression would be zero for every
\(\boldsymbol p_t\). The implementation returns \(\boldsymbol p_t\) in that exact state so that
separation still requires zero tangential pressure. `hueber_stadler_wohlmuth` is the default and
`alart_curnier` selects the degree-one map. This option does not alter mortar dynamics, which uses
separate constraints.

Because the HSW expression has pressure-squared units, its assembled row is divided by the
positive tangential pressure scale \(C_t\). This removes the extra factor under a common pressure
rescaling while retaining the degree-two nonlinear map. Dividing by the solution-dependent
\(w\) would instead reduce HSW to the Alart--Curnier map and is deliberately not done. If user
scales have \(C_n\ne C_t\), the remaining ratio is the requested normal-to-tangential augmentation
ratio rather than an unintended common scaling factor.

## Scale source

The normal and tangential strategy parameters select only the source of \(C_n\) and \(C_t\). Both
strategies use the selected projection map, the same multiplier-equation row compensation, and the
same PETSc right scaling. With `user`, a normalized gap or slip gives \(C=c\); without
normalization the nodal scale is \(C=cM\), so the coefficient multiplying the stored integrated
quantity remains \(c\). With `physical`, the constraint uses the positive
inverse-series-compliance scale computed by the weighted-gap user object. Every installed scale and
mortar weight must be positive and finite.

## Formulation choice

Both maps passed the same unit-scale two- and three-dimensional stick, slip, separation, reversal,
and reclosure checks with physical solutions agreeing within \(10^{-8}\) and common natural-map and
displacement-equilibrium errors below \(10^{-10}\). In the fixed robustness sweep,
Alart--Curnier solved every case with 109 aggregate residual evaluations, compared with 117 for
HSW. Alart--Curnier required one more iteration and residual evaluation in the canonical 2-D
sliding case, but one fewer of each in sticking and two fewer nonlinear iterations in an existing
3-D physical-scale case.

On the unit-scale problem, separate PETSc SVD diagnostics measured initial/active global Jacobian
condition numbers of approximately \(76/47\) for Alart--Curnier and \(42/160\) for HSW. The
pressure scales balance units and block magnitudes; they do not imply an order-one condition number.
The HSW factor \(w/C_t\) can still become small near contact-state transitions and attenuate the
raw nonlinear residual. The broader analyst workloads give mixed results across the two maps, so
`hueber_stadler_wohlmuth` remains the default for compatibility and `alart_curnier` is an explicit
alternative. Iteration counts characterize these benchmarks and are not regression requirements.

The nodal weighted tangential velocity is

\begin{equation}
\widetilde{\boldsymbol v}_{t,j} =
\int_{\gamma_c^{(1)}} \Phi_j \boldsymbol v_{t,h}\,dA,
\end{equation}

and its time integral supplies the weighted tangential displacement used above. The parent
[ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md) constraint
enforces the normal complementarity condition.

!syntax parameters /Constraints/ComputeFrictionalForceLMMechanicalContact

!syntax inputs /Constraints/ComputeFrictionalForceLMMechanicalContact

!syntax children /Constraints/ComputeFrictionalForceLMMechanicalContact
