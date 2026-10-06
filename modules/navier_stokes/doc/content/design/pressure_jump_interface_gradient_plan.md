# Rigorous pressure-jump interface-gradient implementation plan

## Outcome

Replace the single interpolated face gradient used by
[LinearFVPressureCorrectionDiffusionJump](LinearFVPressureCorrectionDiffusionJump.md)
with a conservative, one-sided interface formulation. The completed implementation must represent
separate pressure traces and gradients on the two sides of a baffle, preserve the prescribed jump,
assemble one equal-and-opposite face flux, and remain consistent with the optional two-term
transmissibility.

This is a design and implementation plan, not a description of current behavior.

The change is successful when all of the following are true:

- no baffle calculation assumes that a unique physical pressure gradient exists at the interface;
- the element and neighbor use separate one-sided pressure traces and gradients;
- the baffle pressure jump and normal-flux conservation hold simultaneously;
- anisotropic and nonorthogonal corrections are constructed from one-sided solution gradients,
  not from the reconstructed Rhie-Chow coupling gradient;
- the reconstructed coupling gradient is used only where the two-term secant construction needs
  it;
- the matrix and right-hand side use exactly the same face transmissibility and explicit
  correction;
- the element and neighbor face contributions remain exact negatives of one another;
- results are unchanged in the continuous, orthogonal, isotropic limit; and
- manufactured solutions demonstrate convergence for spatially varying jumps, discontinuous
  mobility, anisotropy, and nonorthogonal baffles.

## Why the current face-gradient average is insufficient

Let the face normal $\boldsymbol n_f$ point from cell $P$ to cell $N$. A pressure baffle has two
traces,

\begin{equation}
  p_f^P,\qquad p_f^N,\qquad p_f^P-p_f^N=J_P.
\label{eq:plan-pressure-traces}
\end{equation}

It may also have two different gradients,

\begin{equation}
  \boldsymbol g_f^P=\boldsymbol\nabla p^P,
  \qquad
  \boldsymbol g_f^N=\boldsymbol\nabla p^N.
\label{eq:plan-one-sided-gradients}
\end{equation}

There is no unique physical quantity called $\boldsymbol\nabla p_f$ at this discontinuity. For a
face-tangent vector $\boldsymbol t_f$,

\begin{equation}
  \boldsymbol t_f\mathbin{\cdot}
  \left(\boldsymbol g_f^P-\boldsymbol g_f^N\right)
  =\boldsymbol t_f\mathbin{\cdot}\boldsymbol\nabla_\Gamma J_P.
\label{eq:plan-tangential-jump-gradient}
\end{equation}

The tangential gradients agree only when the jump is constant along the interface. Normal
gradients can differ even for a constant jump, particularly when the pressure mobility is
discontinuous.

The inherited anisotropic diffusion implementation currently forms one geometrically interpolated
gradient and contracts it with a face tensor. The two-term path instead interpolates cell
coefficient-gradient products. Neither operation represents two explicit interface traces. The
rigorous formulation below replaces those baffle-face operations; ordinary non-baffle faces retain
the inherited discretization.

## Target face equations

The implementation should first be written and tested from a single orientation convention. Define

\begin{equation}
  \boldsymbol h_P=\boldsymbol x_f-\boldsymbol x_P,
  \qquad
  \boldsymbol h_N=\boldsymbol x_N-\boldsymbol x_f,
\label{eq:plan-half-cell-vectors}
\end{equation}

so that both half-cell vectors point nominally from $P$ toward $N$. Let

\begin{equation}
  \delta_P=\boldsymbol h_P\mathbin{\cdot}\boldsymbol n_f>0,
  \qquad
  \delta_N=\boldsymbol h_N\mathbin{\cdot}\boldsymbol n_f>0.
\label{eq:plan-half-cell-normal-distances}
\end{equation}

For a diagonal, symmetric pressure-mobility tensor $\boldsymbol D_s$ on side
$s\in\{P,N\}$, define

\begin{equation}
  \alpha_s=\boldsymbol n_f^T\boldsymbol D_s\boldsymbol n_f,
  \qquad
  \tau_s=\frac{|S_f|\alpha_s}{\delta_s}.
\label{eq:plan-half-cell-conductance}
\end{equation}

The side-specific anisotropic and nonorthogonal correction vector is

\begin{equation}
  \boldsymbol c_s=
  \left(\boldsymbol D_s\boldsymbol n_f-\alpha_s\boldsymbol n_f\right)
  +\alpha_s\left(
    \boldsymbol n_f-\frac{\boldsymbol h_s}{\delta_s}
  \right).
\label{eq:plan-side-correction-vector}
\end{equation}

When nonorthogonal correction is disabled, omit the second parenthesized term. Define the
common-orientation explicit corrections

\begin{equation}
  r_s=|S_f|\boldsymbol c_s\mathbin{\cdot}\boldsymbol g_f^s.
\label{eq:plan-side-explicit-correction}
\end{equation}

With $q_f$ positive from $P$ to $N$, the two half-cell flux equations are

\begin{equation}
\begin{aligned}
  q_f&=\tau_P(p_P-p_f^P)-r_P,\\
  q_f&=\tau_N(p_f^N-p_N)-r_N.
\end{aligned}
\label{eq:plan-half-cell-fluxes}
\end{equation}

Eliminating the two traces with [eq:plan-pressure-traces] gives

\begin{equation}
  T_f=\left(\frac{1}{\tau_P}+\frac{1}{\tau_N}\right)^{-1},
  \qquad
  R_f=T_f\left(\frac{r_P}{\tau_P}+\frac{r_N}{\tau_N}\right),
\label{eq:plan-interface-transmissibility-and-correction}
\end{equation}

and therefore

\begin{equation}
  q_f=T_f\left[(p_P-p_N)-J_P\right]-R_f.
\label{eq:plan-conservative-interface-flux}
\end{equation}

This equation supplies one flux to both control volumes. Its local linear-system contribution is

\begin{equation}
  \boldsymbol A_f=T_f
  \begin{bmatrix}
     1 & -1\\
    -1 &  1
  \end{bmatrix},
  \qquad
  \boldsymbol b_f=
  \begin{bmatrix}
    R_f+T_fJ_P\\
   -R_f-T_fJ_P
  \end{bmatrix}.
\label{eq:plan-interface-local-system}
\end{equation}

Before implementation, verify every sign in [eq:plan-half-cell-fluxes] by swapping $P$ and $N$
and reversing $\boldsymbol n_f$. The resulting physical flux must be unchanged and its stored
orientation must reverse.

## One-sided gradient reconstruction

A first-order cell-centered method may retain one gradient vector per cell, but it must construct
that vector with the correct trace on each face of the cell. In particular, the two cells adjacent
to a baffle must not share one pressure value in their Green-Gauss surface sums. Introduce a
Navier-Stokes-level jump-aware solution-gradient provider rather than adding pressure-jump knowledge
to the framework
[FVGreenGaussGradient](FVGreenGaussGradient.md).

For a baffle face, the provider supplies the pair

\begin{equation}
  \left(\boldsymbol g_f^P,\boldsymbol g_f^N\right)
\label{eq:plan-face-gradient-pair}
\end{equation}

by reading the separately reconstructed gradients of cells $P$ and $N$. It should cache gradient
component vectors by cell and associate them with the pressure and jump generation from which they
were formed. Any optional trace cache is indexed by face ID. All state is invalidated on mesh
changes, time-step retries, recovery, and incompatible jump-generation changes.

Use an interface-aware Green-Gauss fixed-point reconstruction:

1. Treat every pressure-jump face as two coincident one-sided faces during each adjacent cell's
   Green-Gauss surface sum.
2. Use the ordinary boundary-condition value on external boundaries and the ordinary common face
   interpolation on continuous internal faces.
3. On a baffle, calculate separate traces from the lagged half-cell flux equations:

   \begin{equation}
   \begin{aligned}
     p_f^P&=p_P-\frac{q_f+r_P}{\tau_P},\\
     p_f^N&=p_N+\frac{q_f+r_N}{\tau_N}.
   \end{aligned}
   \label{eq:plan-traces-from-flux}
   \end{equation}

4. Insert $p_f^P$ only into the $P$ Green-Gauss sum and $p_f^N$ only into the $N$ sum. If one cell
   touches several baffle faces, insert the appropriate trace from every such face before dividing
   the completed sum by volume.
5. Divide each completed surface sum by its coordinate-system-aware cell volume.
6. Re-evaluate [eq:plan-side-explicit-correction] with the new gradients. One update per outer
   pressure corrector is the default Picard strategy; an inner reconstruction iteration should be
   added only if manufactured tests show that outer convergence is insufficient.

The first reconstruction has no lagged $r_P$ or $r_N$. Initialize both to zero, solve the normal
half-cell problem, and publish the resulting one-sided gradients. Do not initialize by averaging a
gradient across the baffle.

The reconstruction must use the pressure solution and jump from the same fixed-point generation.
Store that generation explicitly and assert it when the face-side gradients are read. This avoids
silently combining a newly updated nonlinear jump with gradients reconstructed from the previous
jump.

## Separation from the reconstructed coupling gradient

Keep the two gradient roles distinct:

- the new one-sided solution gradients supply $r_P$, $r_N$, and $R_f$;
- [FVReconstructedPressureGradient](FVReconstructedPressureGradient.md)
  remains the pressure-velocity coupling gradient used by the momentum predictor; and
- when
  [!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_two_term_pressure_expansion)
  is enabled, the reconstructed coupling data may define a lagged secant transmissibility but must
  not replace the one-sided solution gradients in $R_f$.

Prefer the already conservative pressure-gradient contribution produced when
[RhieChowMassFlux](RhieChowMassFlux.md) recomputes the corrected face flux over
constructing another flux by averaging gradients. Cache that contribution with its face-flux
generation and expose a read-only accessor. Let it be $Q_f^\ell$.

Construct the two-term coefficient as

\begin{equation}
  T_f^\ell=
  \frac{Q_f^\ell+R_f^\ell}
       {\Delta p_f^{\mathrm{smooth},\ell}},
\label{eq:plan-rigorous-two-term-transmissibility}
\end{equation}

where the denominator is obtained from two one-sided reconstructed traces or an algebraically
equivalent one-sided Taylor expansion. The same $R_f^\ell$ must appear in the numerator and in the
right-hand side. Do not add $Q_f^\ell$ separately to the right-hand side.

The coefficient remains a frozen Picard coefficient multiplying the current $p_P$ and $p_N$. The
existing finite-and-positive check remains, but the near-zero denominator criterion should be
replaced by a scale-aware cancellation check based on the two one-sided Taylor terms rather than a
fixed dimensional absolute tolerance.

## Proposed code structure

Keep pressure-jump-specific behavior in `modules/navier_stokes`; do not add baffle or
Rhie-Chow dependencies to the framework Green-Gauss implementation.

1. Add a jump-aware pressure-gradient provider with cell-gradient storage, pressure/jump generation
   metadata, and optional one-sided trace storage for diagnostics.
2. Have the provider owned or coordinated by
   [RhieChowMassFlux](RhieChowMassFlux.md). Reuse the existing pressure system,
   mesh face loop, coordinate factors, and gradient-vector layouts.
3. Cache the conservative pressure-gradient flux contribution when `computeFaceMassFlux()` already
   computes it. Do not perform a second pressure-kernel evaluation merely to recover the same flux.
4. Add a baffle-only helper to
   [LinearFVPressureCorrectionDiffusionJump](LinearFVPressureCorrectionDiffusionJump.md)
   that reads the two adjacent jump-aware cell gradients and returns one structure containing
   $\tau_P$, $\tau_N$, $r_P$, $r_N$, $T_f$, and $R_f$. Compute it once per face assembly and reuse
   it for both matrix and right-hand-side calls.
5. Leave non-baffle internal faces and all ordinary boundary faces on the inherited
   `LinearFVAnisotropicDiffusion` path.
6. Use the one-sided interface data in both the baseline and two-term baffle paths. A disabled
   [!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_two_term_pressure_expansion)
   must disable only the secant replacement, not the one-sided interface correction.
7. Extend restartable state only where the cached generation must survive restart or recovery.
   Prefer recomputation from restored pressure and jump state when it is deterministic and cheap.

Do not add a user-selectable interpolation parameter in the first implementation. There should be
one mathematically defined interface scheme. Add configurability only if verification demonstrates
a real need for more than one closure.

## Solver ordering

The current segregated sequence updates the nonlinear baffle jump in `computeHbyA()`, solves the
pressure corrector, reconstructs a conservative pressure-gradient candidate before pressure
relaxation, relaxes pressure, and then publishes gradients for the next predictor.

Change or augment this sequence so the following state tuple is explicit:

\begin{equation}
  \left(p^\ell,J^\ell,\boldsymbol g_{f,P}^\ell,
  \boldsymbol g_{f,N}^\ell,Q_f^\ell\right).
\label{eq:plan-state-tuple}
\end{equation}

The recommended order for each completed pressure corrector is:

1. retain the jump used to assemble that pressure system until the corrected face flux has been
   computed;
2. cache the conservative pressure-gradient flux contribution from that solve;
3. update and relax the nonlinear jump from the corrected face flux for the next outer iteration;
4. relax the pressure solution;
5. reconstruct the one-sided solution gradients from the relaxed pressure and the newly published
   jump;
6. finalize the reconstructed coupling gradient independently; and
7. record a common generation for every field consumed by the next pressure assembly.

Audit SIMPLE, PIMPLE/PISO correctors, time-step rejection, restart, recovery, mesh changes, and
multiple pressure correctors. No code should infer consistency merely because two vectors happen to
be nonempty.

## Implementation phases and verification gates

### Phase 1: Freeze the mathematical contract

- Implement a standalone test helper for [eq:plan-half-cell-fluxes] through
  [eq:plan-interface-local-system].
- Verify face-orientation reversal, the continuous-coefficient limit, an orthogonal isotropic face,
  a nonorthogonal isotropic face, and an orthogonal anisotropic face.
- Decide and document how zero or invalid half-cell conductances fall back. The fallback must use
  one common transmissibility in the matrix and jump source.

Gate: no production assembly changes until the local algebra passes orientation and conservation
tests.

### Phase 2: Produce one-sided traces and gradients

- Add the jump-aware solution-gradient provider and state-generation tracking.
- Reuse coordinate-aware face areas and cell volumes from the existing Green-Gauss loops.
- Add separate trace contributions for the two sides of every baffle.
- Validate the startup, mesh-change, restart, and recovery paths.

Gate: manufactured affine fields reproduce both one-sided gradients to roundoff on an aligned mesh
and converge at the expected order on skew meshes.

### Phase 3: Replace the baseline baffle correction

- Assemble half-cell conductances and side corrections.
- Eliminate the traces to obtain one conservative $T_f$ and $R_f$.
- Insert $+T_f,-T_f,-T_f,+T_f$ in the local matrix and
  $R_f+T_fJ_P,-R_f-T_fJ_P$ in the right-hand side.
- Retain the inherited operator for non-baffle faces.

Gate: local conservation is at roundoff, orientation reversal is invariant, and the continuous
limit matches `LinearFVAnisotropicDiffusion`.

### Phase 4: Reconnect the two-term path

- Cache and expose the conservative lagged pressure-gradient flux.
- Form the scale-aware smooth pressure drop from one-sided reconstructed data.
- Compute [eq:plan-rigorous-two-term-transmissibility].
- Use the same one-sided $R_f$ in its numerator and in the assembled right-hand side.
- Preserve startup and invalid-coefficient fallback behavior.

Gate: substituting the lagged state into the assembled face equation reproduces $Q_f^\ell$ to
roundoff for every manufactured face.

### Phase 5: Integrate the solver lifecycle

- Assign explicit generations to jumps, one-sided gradients, and cached pressure fluxes.
- Update the SIMPLE/PIMPLE ordering.
- Add assertions for stale or mismatched generations.
- Exercise retry, restart, recovery, and mesh-change invalidation.

Gate: a converged solution is unchanged by an additional pressure corrector, restart/recovery
reproduces the uninterrupted result, and stale state produces a clear diagnostic in debug builds.

### Phase 6: Documentation and cleanup

- Update
  [LinearFVPressureCorrectionDiffusionJump](LinearFVPressureCorrectionDiffusionJump.md)
  with the one-sided derivation, matrix/RHS split, startup behavior, and limitations.
- Document any new object as a regular MOOSE object page with working input syntax.
- Remove only code and comments made obsolete by this implementation.

Gate: MooseDocs builds the changed pages without broken syntax commands or links.

## Required tests

Add tests that isolate mechanisms rather than relying only on integral channel results.

1. **Constant-jump continuous mobility:** inclined baffle, affine pressure on both sides, equal
   tangential gradient, and nonzero normal gradient. This must reduce to the continuous-gradient
   result.
2. **Tangentially varying jump:** prescribe affine one-sided fields whose jump varies along the
   baffle. Verify the two distinct tangential gradients and the exact face flux.
3. **Discontinuous mobility:** choose $\boldsymbol D_P\ne\boldsymbol D_N$ with different normal
   gradients but a continuous normal flux.
4. **Anisotropic mobility:** use diagonal tensors and an inclined face so the tangential anisotropic
   correction is nonzero.
5. **Nonorthogonal mesh:** offset the cell centers relative to the face normal and verify the
   one-sided nonorthogonal corrections.
6. **Combined case:** varying jump, discontinuous anisotropic mobility, and a nonorthogonal baffle.
7. **Orientation reversal:** construct the same physical case with reversed face ownership and
   compare physical fluxes and solutions.
8. **Multiple baffle faces per cell:** use nonparallel baffle faces to prove that every baffle
   contributes its own one-sided trace to the cell gradient.
9. **Degenerate secant state:** zero and cancellation-dominated smooth pressure drops must use the
   documented fallback without creating a non-finite or negative matrix coefficient.
10. **Existing regressions:** retain the one-dimensional form-loss, variable-density, and diagonal
    baffle cases. Do not update gold data until differences are explained by the new interface
    discretization.

Run parallel-process, threaded, distributed-mesh, restart, recovery, and restep variants through
TestHarness options on the same applicable tests rather than creating duplicate test specifications.

For manufactured cases, check more than the final pressure field:

- both one-sided face traces;
- both one-sided gradients;
- $r_P$, $r_N$, $T_f$, and $R_f$;
- the oriented pressure-gradient flux;
- element-plus-neighbor residual cancellation;
- observed mesh-refinement order; and
- invariance under face-orientation reversal.

## Anticipated files

The exact names should follow the smallest design that survives Phase 1, but the expected scope is:

- a new Navier-Stokes jump-aware pressure-gradient provider under `include/fvgradientmethods` and
  `src/fvgradientmethods`, or an equivalently scoped utility if it does not need MooseObject
  lifecycle services;
- `RhieChowMassFlux` and `PorousRhieChowMassFlux` for jump/flux generation and conservative flux
  access;
- `LinearFVPressureCorrectionDiffusionJump` for the half-cell interface operator and assembly;
- `LinearAssemblySegregatedSolve` for state ordering;
- the Navier-Stokes action only if automatic object creation is required;
- focused unit or regression inputs under the existing linear-segregated baffle tests; and
- the object and design documentation.

Avoid modifying `LinearFVAnisotropicDiffusion` unless a pressure-jump-independent extension is
clearly reusable. Its default behavior for continuous fields should not change as a side effect of
this work.

## Risks and controls

- **Circular trace-gradient dependence:** use a clearly lagged Picard update first. Add an inner
  iteration only from convergence evidence.
- **Mismatched nonlinear state:** track explicit generations and validate them at every read.
- **Loss of conservation:** compute one oriented face flux once and add its negative to the other
  row; never assemble side fluxes independently.
- **Ill-conditioned half-cell geometry:** validate $\delta_P$, $\delta_N$, $\tau_P$, and $\tau_N$
  before division and define one documented fallback.
- **Poor Green-Gauss behavior on highly skewed cells:** test multiple baffle faces and
  boundary-adjacent baffles. If the trace iteration cannot recover the required order, replace the
  cell reconstruction with a jump-aware weighted least-squares method rather than crossing the
  jump.
- **Limiter contamination:** initially require an unlimited solution-gradient reconstruction. A
  limiter that acts on a discontinuous cell field needs a separate jump-aware design.
- **Performance:** cache gradient data once per generation and reuse existing face loops and ghosted
  vector layouts. Measure additional storage and assembly time before considering optimization.
- **Overfitting existing gold data:** manufactured identities, conservation, and convergence decide
  correctness; existing regression values only measure compatibility.

## Completion criteria

The work is complete only when the implementation, tests, and documentation establish all of the
following:

- one-sided traces satisfy $p_f^P-p_f^N=J_P$;
- half-cell fluxes agree to the test tolerance;
- matrix and right-hand-side face contributions are conservative;
- the baseline and two-term paths use the same one-sided $R_f$;
- the two-term lagged substitution reproduces its cached conservative pressure flux;
- no baffle correction interpolates $\boldsymbol g_f^P$ and $\boldsymbol g_f^N$ into a purported
  physical interface gradient;
- continuous-field and orthogonal-isotropic limits recover the existing operator;
- manufactured convergence rates meet their design order; and
- all focused and existing baffle regression tests pass in the established MOOSE environment.
