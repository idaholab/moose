# Spring elements

The spring element family of the solid mechanics module connects two nodes with a force that depends on their relative motion. A spring is a two-node line element (`EDGE2`) of any length; its law is a separate material, so the same element, kernel and frame serve a linear spring, a nonlinear elastic or hysteretic spring, a Mohr-Coulomb interface and a dashpot, and a new law is one new material. The family works in the small-displacement setting of the module: the frame of the spring is fixed and the laws act on the relative displacements and rotations of its two nodes.

!alert note
A spring between coincident positions, the zero-length element of other codes, is modeled as a short element with `local_axis_1`: the length of the element enters no law, and the finite element map of the mesh library rejects an element of zero length; see [Springs between coincident positions](#zero-length).

## Objects

| Object | Role |
|---|---|
| [ComputeSpringDeformation](/ComputeSpringDeformation.md) | local frame of the element; relative translation and rotation of node 1 with respect to node 0 in that frame, and their rates |
| [LinearSpring](/LinearSpring.md) | one constant stiffness per local axis |
| [NonlinearElasticSpring](/NonlinearElasticSpring.md) | a force-deformation function per local axis, the same on loading and unloading |
| [MasingHystereticSpring](/MasingHystereticSpring.md) | a backbone per local axis with the Masing rule on unloading and reloading |
| [MohrCoulombSpring](/MohrCoulombSpring.md) | interface: compression, tension up to a tensile strength, elastic shear capped by the Coulomb limit, separation with or without a permanent breakage |
| [ViscousSpring](/ViscousSpring.md) | a damping coefficient per local axis on the rate of the deformation |
| [StressDivergenceSpring](/StressDivergenceSpring.md) | residual and Jacobian of the two nodes for one of the six degrees of freedom |

A law acts either on the three relative translations (its default mode, reporting `spring_force` and `spring_stiffness`) or on the three relative rotations (`mode = rotational`, reporting `spring_moment` and `spring_rotational_stiffness`). A spring with six degrees of freedom takes one law of each mode on the same block. Several springs on one block, a spring in parallel with a dashpot say, take a `base_name` each on their deformation material, their laws and their kernels, which prefixes every property name as in the rest of the module. The Mohr-Coulomb law is translational only: the moment capacity of an interface comes from several translational springs across it, not from a rotational law on one of them.

## Element frame

Local axis 1, $\mathbf{e}_1$, runs from node 0 to node 1, or along `local_axis_1` when that direction is given. Local axis 2, $\mathbf{e}_2$, is the projection on the plane normal to $\mathbf{e}_1$ of `local_axis_2`, or of the global Y axis by default; when the reference direction is along $\mathbf{e}_1$ (within 0.05 degrees) a given `local_axis_2` is an input error and the default falls back to the global axis least aligned with $\mathbf{e}_1$. Axis 3 closes the right-handed frame, $\mathbf{e}_3 = \mathbf{e}_1 \times \mathbf{e}_2$. The rotation matrix $\mathbf{R}$ with the three axes as rows takes a global vector to the local frame. A spring along the global X axis with the default reference has its local axes along X, Y and Z.

## Deformation

The deformation of the spring is the relative displacement of node 1 with respect to node 0 in the local frame, and the relative rotation likewise:

\begin{equation}
\boldsymbol{\delta} = \mathbf{R} \left( \mathbf{u}_1 - \mathbf{u}_0 \right), \qquad \boldsymbol{\vartheta} = \mathbf{R} \left( \boldsymbol{\theta}_1 - \boldsymbol{\theta}_0 \right).
\end{equation}

A positive $\delta_1$ is an elongation along axis 1, and a positive force along axis 1 is a tension. A ground spring is an element whose node 1 is held by Dirichlet conditions: its node 0 moving toward node 1 compresses the spring. With `compute_rates = true` the material also reports the rates $\dot{\boldsymbol{\delta}}$ and $\dot{\boldsymbol{\vartheta}}$ from the time derivative of the solution, together with the derivative of the rate with respect to the displacement of the time integrator, for the viscous law.

## Laws

Every law returns the force $\mathbf{f}$ (or moment) in the local frame and the tangent $\mathbf{k} = \partial \mathbf{f} / \partial \boldsymbol{\delta}$, both multiplied by the tributary area $A$ (default 1). With $A$ the area of the interface the spring represents, the coefficients of the law are given per unit area: a stiffness in force per length cubed (a subgrade modulus), a strength in force per area. One law instance serves a point spring, a bed of springs and a pile shaft, with the area as the only difference.

### Linear

$f_i = k_i \, \delta_i$ with one stiffness $k_i$ per local axis, zero or positive; a zero stiffness leaves that axis free.

!media media/solid_mechanics/spring_law_linear.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_spring_linear
      caption=The linear law: the force along a local axis against the deformation along it, tension positive.

### Nonlinear elastic

$f_i = F_i(\delta_i)$ with $F_i$ a function of the deformation along axis $i$ (the x coordinate of a [PiecewiseLinear](/PiecewiseLinear.md) function with `axis = x`, or of a [ParsedFunction](/MooseParsedFunction.md)), the same curve on loading and unloading, and the tangent its slope. One-sided behaviour is written in the function: a tension-only spring has a zero branch for negative deformations, a compression-only spring for positive ones, and a gap a zero branch between the two deformations that close it. On its open side such a law gives its degree of freedom no stiffness, so under a load that degree of freedom needs another object holding it (a parallel spring through `base_name`, or a prescribed value).

!media media/solid_mechanics/spring_law_nonlinear_elastic.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_spring_nonlinear_elastic
      caption=The nonlinear elastic law: the backbone function of one axis, with the compression-only recipe (a zero branch on the tension side) dashed.


### Masing hysteretic

The first loading follows the backbone $F_i(\delta_i)$, which is assumed odd in the deformation. A change of the loading direction at the converged point $(\delta_r, f_r)$ starts a branch that follows the Masing rule of [!cite](masing1926eigenspannungen), the backbone scaled by two about the reversal point,

\begin{equation}
f = f_r + 2 F\!\left( \frac{\delta - \delta_r}{2} \right),
\end{equation}

whose tangent is $F'((\delta - \delta_r) / 2)$. The branch runs on one side of the backbone, below it when unloading from the backbone and above it when reloading, and ends where it crosses to the other side; from there the backbone is followed again, so a full cycle closes on the backbone and a bilinear backbone gives the loops of linear kinematic hardening (an elastic unloading of twice the yield force). A reversal inside a loop starts a new branch from that point and returns to the backbone where it meets it; the branch it left is not remembered. The state of each axis is its deformation and force at the last reversal, its loading direction and the side of the backbone its branch runs on.

!media media/solid_mechanics/spring_law_masing_hysteretic.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_spring_masing
      caption=The Masing rule on a bilinear backbone: the branch from the reversal point is the backbone scaled by two, and meets the backbone again at the mirrored kink.


The tangent is discontinuous at a reversal, as in every hysteretic law. Under a prescribed deformation this costs nothing. Under a load the Newton iterations of the step after a reversal can jump between the backbone tangent and the branch tangent; the backtracking line search (`line_search = bt`) resolves it, and the load-controlled case of the [verification page](/spring_vandv.md) converges in one or two iterations per step with it.

### Mohr-Coulomb

Local axis 1 is the normal of the interface and the other two its shear directions. The compressive normal stress is

\begin{equation}
n = \sigma_0 - k_n \, \delta_1 ,
\end{equation}

with $\sigma_0$ the `initial_normal_stress` on the interface before any deformation (the horizontal effective stress on a pile shaft, say) and $k_n$ the `normal_stiffness`; $n < -\sigma_t$, with $\sigma_t$ the `tensile_strength` of the bond (zero by default), means the interface has separated and every force is zero. The initial stress enters the Coulomb limit only: it is carried by the soil around the interface, not by the spring, so the normal force of the spring is the closure times the stiffness, $f_1 = k_n \delta_1$ for $\delta_1 < 0$, zero for an opening that relaxes the initial stress, and the tensile part of the normal stress, $f_1 = -n$, once the opening has gone past it, up to the tensile strength. The shear is elastic with the `shear_stiffness` $k_s$ from the plastic slip $\mathbf{s}$ of the previous step, capped by the Coulomb limit on its resultant:

\begin{equation}
\mathbf{t} = k_s \left( \boldsymbol{\delta}_s - \mathbf{s} \right), \qquad |\mathbf{f}_s| \le \begin{cases} c + n \tan\varphi & n \ge 0 \\ c \, (1 + n / \sigma_t) & -\sigma_t \le n < 0 \end{cases}
\end{equation}

with $c$ the `cohesion` and $\varphi$ the `friction_angle`: in tension the limit falls linearly from the cohesion to zero at the tensile strength, so that the bond breaks in shear and in tension together and the capacity has no jump at the separation. When the trial $\mathbf{t}$ exceeds the limit the shear force sits on the limit in the direction of the trial and the slip takes up the rest; the limit is a circle in the shear plane, not a square, so the capacity is the same in every shear direction. On separation the slip is set to the shear deformation, so the shear restarts from zero when the interface closes again, and the bond is back by default; with `permanent_breakage` the interface stays frictional after its first separation, with the `residual_cohesion` (zero by default) and the `residual_friction_angle` (the friction angle by default) and no tensile strength, which a stateful flag of the law records. The tangent of the sliding state has the radial return of the shear block and the change of the limit with the closure, $\partial f_{s,i} / \partial \delta_1 = -k_n \, \mu \, t_i / |\mathbf{t}|$ with $\mu$ the slope of the limit ($\tan\varphi$ in compression, $c / \sigma_t$ in tension), which makes it non-symmetric, as the tangent of a non-associated Mohr-Coulomb solid is; the Newton solve with a direct or an unsymmetric iterative preconditioner handles it.

!media media/solid_mechanics/spring_law_mohr_coulomb.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_spring_mohr_coulomb
      caption=The Mohr-Coulomb interface law: the normal force against the normal deformation, the shear resultant against the shear deformation, and the Coulomb limit against the normal stress with its fall to zero at the tensile strength and the residual line of a permanent breakage.


### Viscous

$f_i = c_i \, \dot{\delta}_i$ with one damping coefficient per local axis, and the tangent $c_i \, \partial \dot{\delta} / \partial \delta$ of the time integrator. A dashpot in parallel with a spring on the same element takes its own `base_name` (the Kelvin-Voigt case of the [verification page](/spring_vandv.md)).

!media media/solid_mechanics/spring_law_viscous.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_spring_viscous
      caption=The viscous law: the force against the rate of the deformation.


## Equilibrium

The spring pulls node 1 toward node 0 with the force and pushes node 0 the other way. [StressDivergenceSpring](/StressDivergenceSpring.md) writes the residual of its component $c$ at the two nodes from the force rotated to the global axes, $\mathbf{f}^g = \mathbf{R}^T \mathbf{f}$, as $r_0 = -f^g_c$ and $r_1 = +f^g_c$, and the Jacobian from the rotated tangent $\mathbf{k}^g = \mathbf{R}^T \mathbf{k} \mathbf{R}$ with the same signs; the off-diagonal blocks couple the three translations among themselves and the three rotations among themselves, never a translation with a rotation. The residual is written directly at the nodes, as the truss kernel does, without quadrature.

## Springs between coincident positions id=zero-length

The mesh library builds the map of every element before any material runs, and the map of a line element has its half-length for Jacobian, so an element of zero length stops the run with a degenerate-map error, however the spring is written; the deformation material reports such an element in its own words. Since the length enters none of the laws, a spring between two coincident positions is a short element: the second node is offset by a small fraction of the neighbouring element size, a millionth say, and `local_axis_1` sets the axis of the spring, so that the offset direction is irrelevant. The spring then behaves exactly as a zero-length element would, and its forces are reported by the auxiliary kernels as for any other spring.

## Using the elements

A spring needs the three displacement variables (and the three rotations for a rotational law), [ComputeSpringDeformation](/ComputeSpringDeformation.md), one or two laws, and one [StressDivergenceSpring](/StressDivergenceSpring.md) per degree of freedom on the spring block. The force of a law is a `RealVectorValue` material property, so [MaterialRealVectorValueAux](/MaterialRealVectorValueAux.md) reports any of its components. Springs share their nodes with the other elements of the mesh: a spring element between a node of a structure and a held node is a ground spring, one between two nodes of two structures a connector.

!listing modules/solid_mechanics/test/tests/spring/static/linear_spring.i block=Materials Kernels

## Limitations id=limitations

- +Small displacements.+ The frame is built on the undeformed element and the laws act on the relative motion of the nodes in it; the spring does not rotate with the structure.
- +Undeformed frame.+ The objects work on the undisplaced mesh (`use_displaced_mesh` is false, where the truss kernel sets it true): with a finite-strain continuum the frame and the lever arms of a spring stay those of the undeformed mesh.
- +No adaptivity.+ A spring element split by mesh adaptivity would be two springs in series at the full stiffness; the deformation material reports a refined spring element, so the spring blocks are kept out of adaptivity.
- +Zero length.+ An element of zero length is rejected by the finite element map of the mesh library; a spring between coincident positions is a short element with `local_axis_1`, see [Springs between coincident positions](#zero-length).
- +Three dimensions.+ The family is written for three displacement variables (and three rotations); a plane model holds its third displacement.
- +Masing memory.+ The hysteretic law remembers the last reversal only: after a reversal inside a loop the branch returns to the backbone, not to the branch it left.
- +Mohr-Coulomb.+ No dilatancy and no softening before the separation: the strength is constant until the interface separates, and the only drop is the breakage of the bond, brittle in the normal direction (the tension falls from the tensile strength to zero at once) and gradual in shear. The breakage is recorded per element, not per step: an interface that separates and closes within one step is not registered as broken.

!bibtex bibliography
