# Rigid-Body Contact: Theory

## Overview

The rigid-body contact system treats one member of the contact pair as
truly rigid and represents it implicitly by a signed-distance (level-set)
function

!equation
g_{\text{LS}} : \mathbb{R}^3 \to \mathbb{R},\qquad
g_{\text{LS}}(x) \begin{cases}
  > 0, & x \text{ outside the rigid body,} \\
  = 0, & x \text{ on the surface,} \\
  < 0, & x \text{ inside the rigid body,}
\end{cases}

with unit outward normal
$\hat{n}(x) = \nabla g_{\text{LS}}(x) / \|\nabla g_{\text{LS}}(x)\|$
available in closed form at any point.  The deformable body enters
through its usual displacement discretization $u_h$.  The contact
interface is the sideset $\Gamma_c$ on the deformable body.

Working with $g_{\text{LS}}$ directly, rather than reconstructing a
paired surface with a geometric search, has three consequences that
shape the rest of the formulation:

- The gap and outward normal are pointwise, exact, and defined even
  where the deformable body has not yet contacted the rigid body.  No
  active set has to be tracked separately from the Newton solve.
- The Jacobian contributions on both branches of the complementarity
  condition are available analytically, which keeps the coupled
  Newton solve well-conditioned.
- A moving rigid body is described by a small number of translation
  and rotation scalars on the *contactor*, not by mesh degrees of
  freedom on a paired surface.

The rest of this page describes each ingredient in turn: the level-set
contactor object family, the pointwise complementarity kernel, the
force-transfer boundary condition, the load-control layer, the outer
Uzawa executioner, the warm-start predictor, and the ancillary
sparsity augmentation.  Standard [`[RigidContact]`](syntax/RigidContact/index.md)
inputs assemble all of these automatically through
[RigidContactAction.md]; the object-level descriptions below are
useful when writing an input by hand or debugging a nonstandard
setup.

## Level-set contactors

The contactor supplies, at any query point $x$, the signed distance
$g_{\text{LS}}(x)$ and unit outward normal $\hat{n}(x)$.  Contactors
inherit from [LevelSetContactor.md], which also carries the
translation and rotation state:

- Cartesian translations $s_x$, $s_y$, $s_z$ that can be prescribed
  as MOOSE `Function`s of time or as unknowns coupled from a scalar
  variable.  The scalar-variable option is what makes force control
  possible --- see below.
- Similar rigid rotations could be added in the future, if required. 

Three concrete contactors ship today:

- [SphereContactor.md] --- analytic sphere with user-supplied center
  and radius.  Signed distance is $\|x - c\| - R$; normal is the unit
  radial vector at $x$.
- [InfiniteCylinderContactor.md] --- analytic infinite cylinder with
  user-supplied axis-origin, axis direction, and radius.  Signed
  distance is the perpendicular distance to the axis minus the radius;
  suitable for rolling-mill / roller-contact setups where the roller
  can be treated as a straight cylinder.
- [SurfaceMeshContactor.md] --- signed distance computed from an
  arbitrary closed, oriented triangulated surface (typically an STL
  file).  Distance is computed by a KDTree lookup of the nearest
  triangle followed by a closest-point-on-triangle test and a
  neighbor sweep.  The sign is derived from the *angle-weighted
  pseudonormal* at the closest surface feature (face interior, edge,
  or vertex) following [!cite](baerentzen2005pseudonormal); this is
  the unique choice that gives the mathematically correct
  inside/outside classification for a consistently oriented closed
  manifold.  Using only the closest triangle's face normal (as a
  more naive implementation would) is fragile at convex corners:
  when the closest surface point coincides with a vertex or edge
  shared by multiple triangles, the KDTree can return a non-adjacent
  triangle whose outward direction disagrees with the true local
  outward and flips the sign of the reported gap.

Both level-set contactors are consumed the same way by the rest of
the stack; downstream kernels see only $g_{\text{LS}}$, $\hat{n}$,
and (through the translation state) their derivatives with respect
to the load-control scalar.

## Normal contact as a nodal complementarity condition

At each node $i$ on the deformable-body contact face $\Gamma_c$, let
$\lambda_i \ge 0$ denote the normal contact Lagrange multiplier
(pressure) and $g_i = g_{\text{LS}}(x_i^{\text{def}})$ the signed
distance from the deformed nodal position to the rigid surface.
Physical no-penetration and complementarity are the pointwise
Karush-Kuhn-Tucker conditions

!equation
g_i \ge 0,\qquad \lambda_i \ge 0,\qquad g_i\,\lambda_i = 0.

The rigid-body contact system enforces these through the min-NCP
reformulation

!equation
\min(c\, g_i,\ \lambda_i) = 0,

assembled node by node by [RigidBodyNodalNCPKernel.md].  The
positive constant $c$ scales the gap so it and the multiplier live
on comparable scales; its concrete role appears in the Jacobian
below.  The two branches of the min are

- Gap branch ($c\,g_i \le \lambda_i$).  The residual is $c\,g_i$;
  its Jacobian is $c\,\partial g_{\text{LS}}/\partial x_i$ against
  the displacement DoFs at node $i$ (i.e. $c\,\hat{n}(x_i)$).
- Lambda branch ($\lambda_i < c\,g_i$).  The residual is
  $\lambda_i$; its Jacobian is the identity on that node's $\lambda$
  DoF.

The min-NCP kernel additionally emits, on the gap branch, the block
that couples $\lambda_i$ to the load-control scalar $s$ when force
control is active (below).  Both branches are analytical, so the
Newton system that combines the min-NCP with the primal residual has
consistent tangents even as nodes cross the branch boundary.

The multiplier variable is bounded pointwise, $0 \le \lambda_i$, by
a small aux system that pairs with PETSc's `SNESVINEWTONSSLS`
bound-constrained solver: the action emits a `bounds_dummy` aux
variable and a `ConstantBounds` aux kernel that enforce
$0 \le \lambda \le 10^{12}$, and the recommended executioner picks
up those bounds.  Users who prefer a plain-Newton solve can
disable the bounds via
[!param](/RigidContact/RigidContactAction/enforce_bounds) on the
action.

## Force transfer to the deformable body

Once the nodal multipliers $\lambda_i$ are known, their traction
contributions must be applied back to the deformable body's
displacement equations.  The transfer is performed by
[RigidBodyNormalMechanicalContact.md], a MOOSE `LowerDIntegratedBC`
acting on the lower-dimensional block that is generated from the
contact sideset by `LowerDBlockFromSidesetGenerator`.  For each
displacement component $d \in \{x, y, z\}$ the boundary condition
assembles

!equation
r_i^{d,\,\text{contact}}
  = -\int_{\Gamma_c} \phi_i\, \lambda\, \hat{n}\cdot\hat{e}_d\ \mathrm{d}\Gamma,

using $\lambda$ interpolated on the lower-d block and $\hat{n}(x)$
evaluated from the contactor at the current (deformed) quadrature
point.  Because $\lambda$ and $u$ live on different blocks (the
lower-d block versus the parent block), the *cross-node* Jacobian
between an LM DoF at one node and a displacement DoF at an
adjacent node on the same lower-d element must be preallocated
explicitly --- MOOSE's default coupling machinery misses that
entry.  [RigidBodyContactSparsity.md] fills the gap and is emitted
automatically by the action.  Skipping this step causes PETSc to
`malloc` during every Jacobian assembly, which is slow and (on some
KSP configurations) can leave the factored preconditioner out of
sync with the actual matrix pattern.

## Load control

For inputs in which the total contact reaction is prescribed rather
than the indenter position, the load-control layer promotes one of
the contactor's translation components to an unknown *scalar*
variable $s$, and closes the system with an integrated force
constraint.  Concretely, if the axis of interest is $\hat{a}$ and
the target reaction is $F(t)$, the constraint is

!equation
R_s = \sum_{i} w_i\, \lambda_i\, (\hat{n}_i \cdot \hat{a}) - F(t) = 0,

assembled by [RigidBodyLoadControl.md].  Weights $w_i$ come from a
companion [NodalArea](/userobjects/NodalArea.md) UO evaluated on the
contact sideset.  The kernel emits the transpose block
$\partial R_{\lambda_j}/\partial s$ from the min-NCP (which the
`NodalKernel` framework has no hook for) so the Jacobian is
symmetric and consistent.

The exact diagonal $\partial R_s / \partial s$ is *structurally
zero* in this formulation (the target $F(t)$ does not depend on
$s$, and the reaction depends on $s$ only through the Schur
complement $K_{ls} K_{ll}^{-1} K_{sl}$, which is not available in
closed form).  Leaving the diagonal zero produces a poorly
conditioned linear solve; PETSc's pivot shift then produces the
right direction *but* the step magnitude is uncontrolled and
Newton overshoots badly on flat contact patches where the true
Schur complement is $O(K_{\text{material}} \cdot A_{\text{contact}})$.
`RigidBodyLoadControl` therefore accepts an optional preconditioning
shift on the $(s, s)$ block through
[!param](/ScalarKernels/RigidBodyLoadControl/kss_stiffness) (a
constant) or
[!param](/ScalarKernels/RigidBodyLoadControl/kss_stiffness_function)
(a function of time).  Only the Jacobian is modified --- the
residual is unchanged, so the physical fixed point $R_s = 0$ is
unchanged; only the intermediate Newton iterates differ.  A
sensible starting value for the constant form is the deformable
body's Young's modulus; the function form is useful when a single
constant does not cover the range spanned by the load history (for
example much larger during initial impact than during a
well-established plastic patch).

The action activates the entire force-control layer when
[!param](/RigidContact/RigidContactAction/force) is set.  It emits
the `NodalArea` UO, the scalar variable
`indenter_<axis>` (customizable), and the `RigidBodyLoadControl`
kernel; the contactor's `disp_<axis>_scalar` should be pointed at
the same scalar name.  When [!param](/RigidContact/RigidContactAction/force)
is unset the load-control objects are skipped entirely.

## The Uzawa outer executioner

For the hardest force-controlled inputs --- typically finite-strain
plasticity with rapidly changing effective stiffness across the
patch --- monolithic Newton on the coupled $(u, \lambda, s)$ system
can stall on the pathological $R_s$ direction.  The
[UzawaTransient.md] executioner splits the solve into a 1D outer
Newton on $s$ wrapping an inner primal solve on $(u, \lambda)$ that
holds $s$ fixed at each outer iterate.  For the inner solve
`RigidBodyLoadControl` is switched (via `setMode`) into a
`PinScalar` mode that emits $R_s = s - s_{\text{pin}},\ K_{ss} = 1$
and no coupling blocks; the primal Newton then converges as a
standard bound-constrained problem.  After the inner solve the
kernel returns to `ForceBalance` mode, the true residual
$R_s(u_\star, \lambda_\star; s)$ is read from
`currentReactionMinusF()`, and $s$ is updated by

!equation
s \leftarrow s - \frac{R_s}{k_{ss}^\text{eff}},\qquad
k_{ss}^\text{eff} = \operatorname{signedKssApprox}(),

subject to a `max_step` trust region.  The outer loop terminates on
$|R_s|$ or a relative reduction against the initial outer residual.
See [UzawaTransient.md] and [RigidBodyLoadControl.md] for the
detailed parameter set; the [Vickers indentation
example](modules/contact/rigid_contact/examples/material_into_indenter.md)
is the canonical use case.

Displacement-controlled runs (no `force` parameter, no
`load_control_kernel` on `UzawaTransient`) fall through the outer
loop and behave identically to a plain `Transient` executioner, so
users can leave `UzawaTransient` in place across model families
without paying a runtime cost.

## Warm-start predictor

Even with the preconditioning shift and the Uzawa split, the
outer solve at each new time step is helped substantially by a
better initial guess than "copy the previous step".  The
[RigidBodyContactPredictor.md] runs a small Newton sub-solve
restricted to the contact region --- the LM DoFs on the sideset,
displacement DoFs within $k$ element hops into the bulk, and
(optionally) the load-control scalar --- to resolve the local
contact subproblem before the full monolithic Newton fires.  The
sub-solve is bounds-aware ($\lambda \ge 0$ by simple projection
after each sub-step) and is entirely local: on failure the entry
state is restored and only the outer SNES is left to do the work.

The predictor is a MOOSE `Predictor`, so it plugs into any
executioner (including `UzawaTransient`) and can be flipped on and
off at runtime with the MOOSE Controls system through the standard
`enable` parameter.  On the shipped force-controlled tests it
typically cuts cumulative nonlinear iterations by a factor of
three to four and halves wall time.

## Sparsity augmentation

[RigidBodyContactSparsity.md] serves two roles beyond the
LM-to-displacement cross-node preallocation described above:

- It registers a `GhostEverything` relationship manager on the
  contact-face elements.  This is needed both because
  `RigidBodyLoadControl` is a `NodalScalarKernel` (called only on
  the rank that owns the scalar DoF, but requiring access to every
  boundary LM value to form the integrated reaction) and because
  the sparsity augmentation itself iterates the lower-d block on
  every rank.  Rigid-contact meshes are small compared to the
  bulk, so full ghosting on the contact face is cheap.
- It registers its augmentation via `SystemBase::addExtraSparsityCallback`
  so it coexists with MOOSE's existing extra-sparsity function on the
  nonlinear-system DofMap without tripping libMesh's "both function
  and object slots set" warning.

Users almost never interact with this object directly; the action
emits it whenever
[!param](/RigidContact/RigidContactAction/add_sparsity_uo) is left at
its default of `true`.

## Recommended input structure

For an ordinary rigid-contact input, the minimum recipe is:

1. A `[Physics/SolidMechanics/QuasiStatic]` block for the deformable
   body (with `add_variables = true`, unless the user is defining
   displacements manually --- see the plastic examples).
2. A `[UserObjects]` entry for the level-set contactor
   ([SphereContactor.md] or [SurfaceMeshContactor.md]).
3. A `[RigidContact][<name>]` sub-block referencing the contactor
   and naming the contact sideset.
4. Optionally, a `force` parameter on the sub-block to activate
   force control; a `[Predictor]` of type
   `RigidBodyContactPredictor` inside the executioner; and/or a
   switch to [UzawaTransient.md] for the outer loop.

See the [worked examples](modules/contact/rigid_contact/index.md#worked-examples)
for the four ways this fits together in practice.

## References

!bibtex bibliography
