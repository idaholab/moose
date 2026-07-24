# Physics-Balanced Mortar Contact Scaling

## Purpose

Mortar contact Lagrange multipliers remain physical normal and tangential pressures in the
solution, restart data, outputs, and all multiphysics consumers. The `user` and `physical`
strategies select only the source of the positive nodal pressure scales. Both strategies use the
same NCP residual, multiplier-row compensation, and solver-side right scaling, without changing
the physical zero set or multiplier value semantics.

For a physical Jacobian \(J\), PETSc solves

\[
(J D)y=b, \qquad x=D y.
\]

MOOSE supplies \(D=C\) on contact multiplier degrees of freedom and \(D=1\) elsewhere. Contact
also compensates the multiplier equation rows for MOOSE variable scaling. The effective active
normal-contact operator is therefore approximately

\[
S J D \approx
\begin{bmatrix}
s_u K & s_u B^T C\\
s_u C B & 0
\end{bmatrix}.
\]

Near-symmetry applies to this effective \(SJD\) operator. The physical Jacobian stored by MOOSE is
not permanently column-scaled.

## Three Independent Scales

The implementation keeps three concepts separate:

- \(C\) is a positive right/column scale with units of stiffness per normal length. It
  converts a solver correction into a physical pressure correction. It is also used in the NCP
  augmentation terms. The physical strategy derives \(C=Q\); the user strategy supplies it.
- \(M_i\) is the accumulated mortar surface weight for one multiplier degree of freedom. It only
  normalizes integrated gaps and slips, for example \(\bar g_i=G_i/M_i\). It does not enter the
  physical traction. For user constants, \(C=c\) with normalized gaps and slips, and \(C=cM_i\)
  otherwise.
- \(s_u/s_\lambda\) compensates the multiplier equation row for MOOSE variable scaling. After
  MOOSE applies the multiplier row scale, the effective row scale is \(s_u\).

## Physical Stiffness

At each mortar quadrature point, the stiffness scale is the inverse series compliance of the two
adjacent volume elements in the reference mesh:

\[
Q_n = \left(\frac{h_s}{C_{nn,s}}+\frac{h_p}{C_{nn,p}}\right)^{-1},
\qquad
h=\frac{\text{interior-parent volume}}{\text{mortar-face measure}},
\]

where

\[
C_{nn}=(n\otimes n):C:(n\otimes n).
\]

There is no factor of two. The quadrature values are accumulated with the existing mortar test
function and integration weight, communicated to the owner of each multiplier degree of freedom,
and divided by their accumulated weight. Stiffnesses, lengths, weights, and final scales must be
finite and positive.

\(Q\) is an algorithmic `Real`, not an AD constitutive value. It is refreshed during initial
execution, at timestep start, and after a mesh change, and remains frozen through the associated
nonlinear solve. This implementation uses \(Q_t=Q_n\) in both tangential directions.

## Normal Contact

Physical normal pressure \(p_n\) continues to produce the displacement traction. For a multiplier
degree of freedom \(i\), contact sets \(D_i=C_{n,i}\) and assembles

\[
r_{n,i}=\frac{s_u}{s_n}
\min\left(p_{n,i},C_{n,i}\bar g_i\right).
\]

The implementation stores the raw weighted gap, so its gap coefficient is \(C_{n,i}/M_i\).
All installed scales and mortar weights must be positive and finite. The displacement components
must have a common positive row scaling.

## Coulomb Friction

Each tangential multiplier degree of freedom receives \(D_i=C_{t,i}\). With the convention that
the normalized gap \(\bar g_i\) is positive in separation, define

\[
a_i=p_{n,i}-C_{n,i}\bar g_i, \qquad
R_i=\mu\max(0,a_i), \qquad
q_{t,i}=p_{t,i}+C_{t,i}\bar d_i.
\]

The minus sign in \(a_i\) is essential: opening decreases the augmented normal pressure. The
selectable degree-one Alart--Curnier residual is

\[
F_{\mathrm{AC},i}=p_{t,i}-
\operatorname{Proj}_{\|\cdot\|\le R_i}(q_{t,i}),
\]

and the degree-two Hueber--Stadler--Wohlmuth residual is

\[
F_{\mathrm{HSW},i}=\max(R_i,\|q_{t,i}\|)p_{t,i}-R_iq_{t,i}.
\]

For \(w_i=\max(R_i,\|q_{t,i}\|)>0\),
\(F_{\mathrm{HSW},i}=w_iF_{\mathrm{AC},i}\), so the formulations have the same roots. At
\(w_i=0\), the unguarded degree-two expression loses the condition \(p_{t,i}=0\); the
implementation explicitly returns \(p_{t,i}\) in that exact separated state. The degree-one row is
homogeneous in pressure, while the degree-two row is homogeneous of degree two and is divided by
the tangential pressure scale before assembly.

Friction functions, contact-state checks, Coulomb limits, wear, traction, and outputs continue to
use physical pressure. Physical tangential scaling requires physical normal scaling. The selectable
formulations apply only to quasistatic Coulomb mortar contact; `mortar_dynamics` is unchanged.

## Literature

The degree-one map follows P. Alart and A. Curnier, "A mixed formulation for frictional contact
problems prone to Newton like solution methods," *Computer Methods in Applied Mechanics and
Engineering* 92(3), 353--375 (1991),
[doi:10.1016/0045-7825(91)90022-X](https://doi.org/10.1016/0045-7825(91)90022-X).
The degree-two map and mortar active-set setting follow S. Hueber, G. Stadler, and B. I. Wohlmuth,
"A primal-dual active set algorithm for three-dimensional contact problems with Coulomb friction,"
*SIAM Journal on Scientific Computing* 30(2), 572--596 (2008),
[doi:10.1137/060671061](https://doi.org/10.1137/060671061), and M. Gitterle et al.,
"Finite deformation frictional mortar contact using a semi-smooth Newton method with consistent
linearization," *International Journal for Numerical Methods in Engineering* 84(5), 543--571
(2010), [doi:10.1002/nme.2907](https://doi.org/10.1002/nme.2907).

## PETSc KSP Behavior

`KSPSetRightDiagonalScale()` retains a reference to a runtime vector. `NULL` clears it, and
`KSPGetRightDiagonalScale()` returns the borrowed vector. Each entry must be finite and nonzero.
An entry may be refreshed between mortar assemblies, as required by an unnormalized user scale
\(C=cM_i\); multiple contributors within one assembly must agree. Reinstalling an unchanged value
does not change the vector object state.
The reciprocal is cached until the vector object state changes.

For `KSPSolve()`, pre-solve callbacks see physical matrices, right-hand side, and initial guess.
PETSc then temporarily right-scales both the operator and preconditioning matrix, avoiding a
duplicate operation when they alias. Nonzero physical guesses are mapped by \(D^{-1}\). Existing
KSP and PC types operate on \(AD\) and \(PD\), including direct factorization, AMG, field split,
and matrix-free operators that support managed shell scaling. `PCPostSolve()` runs in scaled
coordinates. PETSc maps the solution by \(D\), restores both matrices and their physical object
states, and then runs KSP guess updates, post-solve callbacks, final views, and final-residual
reporting.

Unchanged matrix and scale object states preserve preconditioner reuse; changing either forces a
rebuild. Matrix restoration is attempted on both normal and error exits. Transpose and multiple
right-hand-side solves currently reject an active right scale.

Attached right and near nullspaces must be invariant under \(D\). Mechanical contact satisfies
this condition because displacement entries have \(D=1\), while mechanical nullspace vectors have
zero multiplier components.

## Compatibility and Symmetry Limits

No multiplier is renamed or converted. Existing pressure outputs, samplers, restarts, checkpoint
data, field splits, heat transfer, wear, and other pressure consumers keep their physical
semantics. Switching between `user` and `physical` strategies across a restart is valid because
both store pressure.

Active normal contact and sticking friction can be nearly symmetric in the effective operator.
Sliding friction, Petrov-Galerkin mortar, geometric terms, and nonsymmetric material tangents can
remain nonsymmetric.
