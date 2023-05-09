# PeriodicSegmentalConstraint

!syntax description /Constraints/PeriodicSegmentalConstraint

## Description

This `Constraint` demonstrates the usage of the scalar augmentation class described in [MortarScalarBase.md].
The other terms in the weak form are handled using the [EqualValueConstraint](/EqualValueConstraint.md)
as described below.

In comparison to Dirichlet or Neumann conditions, periodic boundary conditions have been found
to typically be the most accurate (fastest converging) approach for applying macro-to-micro scale
transition constraints.
Several methods for imposing periodic boundary conditions exist, each with pros and cons.
For example, the mortar approach requires an extra Lagrange multiplier field.
Alternatively, the periodic condition can be imposed by the penalty method using [PenaltyPeriodicSegmentalConstraint.md] or one of the other periodic approaches in `MOOSE`.

This class provides the macro-micro coupling terms to implement periodic boundary conditions
using the mortar method, as proposed within [!cite](reis_mortar_2014). Alternatively, these
equations impose an average value of the diffusive flux of a spatial variable over a domain
using surface rather than volume integrals.

The strong form is posed over domain $\Omega$ with opposing boundary pairs $\Gamma^+$
and $\Gamma^-$:

!equation id=strong-form
\begin{aligned}
  \nabla\cdot D \nabla u = 0 \text{ in } \Omega\\
  \llbracket u \rrbracket = \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket \text{ on } \Gamma^+\\
  D \nabla u(\vec{x}^+) \cdot \hat{n}^+ = \lambda = -D \nabla u(\vec{x}^-) \cdot \hat{n}^- \text{ on } \Gamma^+\\
  \int_{\Gamma^+}{\lambda \llbracket \vec{x} \rrbracket d\Gamma} = V_0 \vec{\sigma}
\end{aligned}

where $\epsilon$ is the average diffusive gradient to be solved for,
$\sigma$ is the imposed average diffusive flux, and $\lambda$ is the
Lagrange multiplier that imposes this average constraint.
The jump operator is defined for a single valued or vector valued field
as $\llbracket u \rrbracket = u^{+} - u^{-}$ and
$\llbracket \vec{x} \rrbracket = \vec{x}^{+} - \vec{x}^{-}$, respectively.

The corresponding weak form is (using inner-product notation):

!equation id=weak-form
\begin{aligned}
  (\nabla \psi, D \nabla u)_\Omega - \langle \llbracket \psi \rrbracket,\lambda \rangle _{\Gamma^+} = 0\\
  -\langle \Phi , \llbracket u \rrbracket \rangle _{\Gamma^+} + \langle \Phi , \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket \rangle _{\Gamma^+} = 0\\
  \langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \lambda \rangle _{\Gamma^+} = \langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \vec{\sigma} \cdot \hat{n} \rangle _{\Gamma^+}
\end{aligned}

where $\psi$ is the test function of the diffusive variable $u$, $\Phi$ is the test function
for the Lagrange multiplier $\lambda$, and $\vec{\kappa}$ is an arbitrary test vector (spatially uniform)
to impose the constraint involving the scalar variable $\vec{\epsilon}$.
As is typical for mixed-field problems with Lagrange multipliers, the shape functions for
$u$ and $\lambda$ need to be chosen to satisfy the Babuska-Brezzi inf-sup condition if
stabilization is not added to the system. As discussed in [!cite](reis_mortar_2014),
using quadratic $u$ and piecewise linear $\lambda$ (discontinuous at corners) provides
for stable results. Note that element-discontinuous (e.g. `L2_LAGRANGE` or `MONOMIAL` basis)
does not produce stable results without interpolation. An easy way to make a discontinuous
$\lambda$ field at corners is described below.

## Input File Parameters

The terms in the weak form [weak-form] are handled by several different classes.
The volume integrals are handled by [`Diffusion`](source/kernels/Diffusion.md) or
[`MatDiffusion`](source/kernels/MatDiffusion.md). The surface terms
$\langle \llbracket w \rrbracket,\lambda \rangle _{\Gamma^+}$ and
$\langle \mu , \llbracket u \rrbracket \rangle _{\Gamma^+}$ are computed by
[EqualValueConstraint.md]. The remaining three terms are handled by this class.

Two of these objects are shown in the input file below:

!listing test/tests/mortar/periodic_segmental_constraint/periodic_simple2d.i block=Constraints

The applied macroscale diffusive flux $\sigma$ is applied as the `sigma` vector via an auxillary
scalar. The computed macroscale diffusive gradient $\epsilon$ is assigned in a scalar variable `epsilon`.
Both of these scalars should have the same number of components as the spatial dimension of $\Omega$.
The volume integral of the gradient of the primary field will be constrained to $\epsilon$
in a weak sense.

Also, the `coupled_scalar` must be assigned the same scalar as `epsilon`.

The microscale diffusion variable is specified using the `primary_variable` parameter.
If the solution values to be matched are between different variables, the
`secondary_variable` parameter can also be supplied.
The enforcement takes place using Lagrange multipliers assigned to `variable`.
These same parameters must be used for the micro-micro coupling terms
in the [EqualValueConstraint](/EqualValueConstraint.md) object.

The generation of the lower-dimensional mesh surfaces for $\Gamma^+$ and $\Gamma^-$
are described in the [`Mortar Constraint system`](syntax/Constraints/index.md). The
projection between two separated surfaces on opposite sides of the domain are naturally
handled by the system. This is true for both `EqualValueConstraint` and
`PeriodicSegmentalConstraint`. In fact, the meshes can be nonconforming as long as
the geometry is conforming, although the choice of $\lambda$ discretization becomes
more delicate. Note that the `periodic` parameter is NOT needed, but if it is applied
then it should be the same for BOTH `EqualValueConstraint` and
`PeriodicSegmentalConstraint`.

!alert note title=Parallel offsets
Due to current restrictions on `AutomaticMortarGeneration`, the opposing surfaces must be
directly opposite along the unit normal direction.

As mentioned above, the $\lambda$ discretization needs to be continuous along patches
of element faces (`LAGRANGE`, not `MONOMIAL`) in order to be stable, but must be discontinuous along
corners of the mesh where the outward unit normal $\hat{n}$ is discontinuous since it is
a flux variable (see the thrid condition [strong-form]). An easy way to do this is to make a
separate `LAGRANGE` variable for each 'face' of the model with different $\hat{n}$, which
usually corresponds with different named side-sets or boundaries used for creating
lower-dimensional mesh surfaces. This approach is demonstrated in many of the test input files.

!alert warning title=Solver Type NEWTON
The `PJFNK` solver does not perform well for discrete systems lacking terms on the diagonal
of the Jacobian matrix, such as this mortar method. Thus, the `NEWTON` solver is recommended.

!syntax parameters /Constraints/PeriodicSegmentalConstraint

!syntax inputs /Constraints/PeriodicSegmentalConstraint

!syntax children /Constraints/PeriodicSegmentalConstraint

!bibtex bibliography
