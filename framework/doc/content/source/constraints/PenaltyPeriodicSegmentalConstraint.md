# PenaltyPeriodicSegmentalConstraint

!syntax description /Constraints/PenaltyPeriodicSegmentalConstraint

## Description

This `Constraint` demonstrates the usage of the scalar augmentation class described in [MortarScalarBase.md].
The other terms in the weak form are handled using the [PenaltyEqualValueConstraint](/PenaltyEqualValueConstraint.md)
as described below.

In comparison to Dirichlet or Neumann conditions, periodic boundary conditions have been found
to typically be the most accurate (fastest converging) approach for applying macro-to-micro scale
transition constraints.
Several methods for imposing periodic boundary conditions exist, each with pros and cons.
For example, the periodic constraint will only be satisfied approximately by this method.
Alternatively, the periodic condition can be imposed by the Lagrange multiplier method using [PeriodicSegmentalConstraint.md] or one of the other periodic approaches in `MOOSE`.

This class provides the macro-micro coupling terms to implement periodic boundary conditions
using the penalty method, which is a subset of the Discontinuous Galerkin method
proposed within [!cite](aduloju_primal_2020). Alternatively, these
equations impose an average value of the diffusive flux of a spatial variable over a domain
using surface rather than volume integrals.

The strong form is posed over domain $\Omega$ with opposing boundary pairs $\Gamma^+$
and $\Gamma^-$ is written in [PeriodicSegmentalConstraint.md] for the mortar method.
The corresponding weak form is (using inner-product notation):

!equation id=weak-form
\begin{aligned}
  (\nabla \psi, D \nabla u)_\Omega + \langle \llbracket \psi \rrbracket,\tau \llbracket u \rrbracket \rangle _{\Gamma^+} - \langle \llbracket \psi \rrbracket,\tau \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket \rangle _{\Gamma^+} = 0\\
  -\langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \tau \llbracket u \rrbracket \rangle _{\Gamma^+} +\langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \tau \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket \rangle _{\Gamma^+} = \langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \vec{\sigma} \cdot \hat{n} \rangle _{\Gamma^+}
\end{aligned}

where $\epsilon$ is the average diffusive gradient to be solved for,
and $\sigma$ is the average diffusive flux imposed through the penalty constraint.
Also, $\psi$ is the test function of the diffusive variable $u$,
and $\vec{\kappa}$ is an arbitrary test vector (spatially uniform)
to impose the constraint involving the scalar variable $\vec{\epsilon}$.
The jump operator is defined for a single valued or vector valued field
as $\llbracket u \rrbracket = u^{+} - u^{-}$ and
$\llbracket \vec{x} \rrbracket = \vec{x}^{+} - \vec{x}^{-}$, respectively.
Finally, $\tau$ is a penalty parameter to impose the constraint.

Note: The macro-to-micro constraint $\llbracket u \rrbracket = \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket$
will only be satisfied approximately by this method, depending on the size of the penalty parameter.

## Input File Parameters

The terms in the weak form [weak-form] are handled by several different classes.
The volume integrals are handled by [`Diffusion`](source/kernels/Diffusion.md) or
[`MatDiffusion`](source/kernels/MatDiffusion.md). The surface term
$\langle \llbracket w \rrbracket,\tau \llbracket u \rrbracket \rangle _{\Gamma^+}$ is computed by
[PenaltyEqualValueConstraint.md]. The remaining four terms are handled by this class.

Two of these objects are shown in the input file below:

!listing test/tests/mortar/periodic_segmental_constraint/penalty_periodic_simple2d.i block=Constraints

The applied macroscale diffusive flux $\sigma$ is applied as the `sigma` vector via an auxillary
scalar. The computed macroscale diffusive gradient $\epsilon$ is assigned in a scalar variable `epsilon`.
Both of these scalars should have the same number of components as the spatial dimension of $\Omega$.
The volume integral of the gradient of the primary field will be constrained to $\epsilon$
in a weak sense, depending on the size of the penalty parameter `penalty_value`.

Also, the `coupled_scalar` must be assigned the same scalar as `epsilon`.

The microscale diffusion variable is specified using the `primary_variable` parameter.
If the solution values to be matched are between different variables, the
`secondary_variable` parameter can also be supplied.
These same parameters must be used for the micro-micro coupling terms
in the [PenaltyEqualValueConstraint](/EqualValueConstraint.md) object.

The generation of the lower-dimensional mesh surfaces for $\Gamma^+$ and $\Gamma^-$
are described in the [`Mortar Constraint system`](syntax/Constraints/index.md). The
projection between two separated surfaces on opposite sides of the domain are naturally
handled by the system. This is true for both `PenaltyEqualValueConstraint` and
`PenaltyPeriodicSegmentalConstraint`. In fact, the meshes can be nonconforming as long as
the geometry is conforming, although the choice of `penalty_value` becomes
more delicate. Note that the `periodic` parameter is NOT needed, but if it is applied
then it should be the same for BOTH `PenaltyEqualValueConstraint` and
`PenaltyPeriodicSegmentalConstraint`.

!alert note title=Parallel offsets
Due to current restrictions on `AutomaticMortarGeneration`, the opposing surfaces must be
directly opposite along the unit normal direction.

!syntax parameters /Constraints/PenaltyPeriodicSegmentalConstraint

!syntax inputs /Constraints/PenaltyPeriodicSegmentalConstraint

!syntax children /Constraints/PenaltyPeriodicSegmentalConstraint

!bibtex bibliography
