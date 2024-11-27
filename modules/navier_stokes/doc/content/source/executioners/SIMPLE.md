# SIMPLE

!syntax description /Executioner/SIMPLE

## Overview

This executioner is based on the algorithm proposed by [!cite](patankar1983calculation). The algorithm
is based on the splitting of operators and successive correction for the momentum and pressure fields.
The formulation implemented in MOOSE has been presented in [!cite](jasak1996error) and [!cite](juretic2005error).
See also the examples and derivations in [!cite](moukalled2016finite).
The concept relies on deriving a pressure equation using the discretized form of the momentum
equations together with the continuity constraint. Let's take the steady-state incompressible Navier-Stokes equations
in the following form:

!equation id=momentum-eq
\nabla \cdot \left(\rho \vec{u} \otimes \vec{u}\right) - \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u} +\nabla \vec{u}^T \right)\right) = -\nabla p + \vec{G}.

!equation id=continuity-eq
\nabla \cdot \left(\rho \vec{u}\right) = 0.

Where $\vec{u}$ denotes the velocity, $p$ the pressure, $\rho$ the density, and $\mu_\text{eff}$ the effective dynamic viscosity
which potentially includes the contributions of eddy viscosity derived from turbulence models.
Term $\vec{G}$ expresses a volumetric source term which can be potentially velocity-dependent.
As a first step, we assume that we have a guess for the pressure field, therefore the gradient is known. Furthermore, we assume that
the advecting velocity field is known from the previous iteration. By explicitly showing the iteration index,
[!eqref](momentum-eq) and [!eqref](continuity-eq) become:

!equation id=momentum-eq-iteration
\nabla \cdot \left(\rho \vec{u}^{n-1} \otimes \vec{u}^n\right) - \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u}^n +\nabla \vec{u}^{n,T}\right)\right) = -\nabla p^{n-1} + \vec{G}(\vec{u}^{n-1},\vec{u}^{n}).

!equation id=continuity-eq-iteration
\nabla \cdot \left(\rho \vec{u}^n\right) = 0.

At this point, we should note that the finite volume discretization in MOOSE uses a collocated formulation which has an advantage
of being flexible for unstructured meshes. However, in certain scenarios it can exhibit numerical pressure checker-boarding
due to the discretization of the pressure gradient and continuity terms. A common approach for tackling this issue is the
utilization of the Rhie-Chow interpolation method (See [!cite](rhie1983numerical) and [!cite](moukalled2016finite) for a detailed
explanation). This means that the face velocities (or face fluxes) are determined using pressure corrections. As we will see
later, due to this behavior, the iteration between pressure and velocity will in fact be an iteration between
pressure and face velocity. Nevertheless, to keep this in mind we add a subscript to the advecting velocity in our formulation:

!equation id=momentum-eq-rc
\nabla \cdot \left(\rho \vec{u}^{n-1}_{RC} \otimes \vec{u}^n\right) - \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u}^n +\nabla \vec{u}^{n,T}\right)\right) = -\nabla p^{n-1} + \vec{G}(\vec{u}^{n-1},\vec{u}^{n}).

!equation id=continuity-eq-rc
\nabla \cdot \left(\rho \vec{u}^n_{RC}\right) = 0.

Next, we split the operator acting on $\vec{u}$ in the momentum equation into two components: a component that incorporates effects
that result in contributions to the diagonal of a soon-to-be-generated system matrix and another component that contains
everything else. With this in mind, we can rewrite the equation the following, semi-discretized way:

!equation id=momentum-eq-semi-discretized
A(\vec{u}^{n-1})\vec{u}^n + H(\vec{u}^{n}) = -\nabla p^{n-1},

where $A(\vec{u}^{n-1})$ is the diagonal contribution, and $H(\vec{u}^{n-1})$ includes the off-diagonal contributions
multiplied by the solution together with any additional volumetric source and sink terms (i.e. the discretized forms of $\vec{G}$).
One can solve this equation to obtain a new guess for the velocity field. This guess, however, will not respect the
continuity equation, therefore we need to correct it. For this, a pressure equation is derived from the following formulation:

!equation id=pressure-eq-start
A\vec{u}^n + H(\vec{u}^{n}) = -\nabla p^n.

By applying the inverse of the diagonal operator (a very cheap process computationally), we arrive to the following expression:

!equation id=pressure-eq-ainv
\vec{u}^n + A^{-1}H(\vec{u}^{n}) = -A^{-1}\nabla p^n,

By applying the continuity equation onto $\vec{u}^n$ (which is a constraint) and assuming that the Rhie-Chow
interpolation is used for the velocity, we arrive to a Poisson equation for pressure:

!equation id=pressure-eq-poisson
\nabla \cdot \left(A^{-1}H(\vec{u}^{n})\right) = -\nabla \cdot \left(A^{-1}\nabla p^n\right).

This equation is solved for a pressure which can be used to correct the face velocities in a sense that they
respect the continuity equation. This correction already involves a Rhie-Chow interpolation, considering that
the $A^{-1}H$ and $A^{-1}$ fields are interpolated to the faces in a discretized form:

!equation id=pressure-eq-correction
\vec{u}^{n+1}_{RC,f} = - \left(A^{-1}H(\vec{u}^{n})\right)_f - (A^{-1})_f \nabla p^n_f.

This correction applies the continuity constraint in an iterative manner, while ensuring the lack of
numerical pressure checker-boarding phenomena.

The next guess for the velocity, however, does not necessarily respect the momentum equation. Therefore,
the momentum prediction and pressure correction steps need to be repeated until both the momentum and
continuity equations are satisfied.

!alert! note

The iterative process above is not stable if the full update is applied every time. This means that the
variables need to be relaxed. Specifically, it is a common practice to relax the pressure when plugging it
back to the gradient term in the momentum predictor:

!equation id=pressure-relaxation
p^{n+1,*} = p^n + \lambda_p (p^{n+1}-p^n),

where $p^{n+1,*}$ is the relaxed field and $\lambda_p \in (0,1]$ is the corresponding relaxation parameter.

!alert-end!

!alert! note

To help the solution process of the linear solver, we add options to ensure diagonal dominance through
the relaxation of equations. This is done using the method mentioned in [!cite](juretic2005error), meaning that
a numerical correction is added to the diagonal of the system matrix and the right hand side. This is
especially useful for advection-dominated systems.

!alert-end!

!alert! note

Currently, this solver only respects the following `execute_on` flags: `INITAL`, `TIMESTEP_BEGIN`, and `FINAL`, other flags are ignored. `MultiApps` and the corresponding `MultiappTransfers` are executed at `FINAL` only.

!alert-end!

## Example Input Syntax

The setup of a problem with the segregated solver in MOOSE is slightly different compared to
conventional monolithic solvers. In this section, we highlight the main differences.
For setting up a 2D simulation with the SIMPLE algorithm, we need three linear systems in MOOSE:
one for each momentum component and another for the pressure. The different systems
can be created within the `Problem` block:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-velocity-pressure.i block=Problem

It is visible that we requested that MOOSE keeps previous solution iterates as well. This is necessary to
facilitate the relaxation processes mentioned in the overview. Next, we create linear FV variables and assign them to the
given systems.

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-velocity-pressure.i i block=Variables

The kernels are then created within the `LinearFVKernels` block. The fundamental terms that contribute to the
face fluxes in the momentum equation (stress and advection terms) are lumped into one kernel. Furthermore,
instead of adding contribution from the continuity equation, we build an anisotropic diffusion (Poisson) equation for
pressure:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-velocity-pressure.i block=LinearFVKernels

By default, the coupling fields corresponding to $A^{-1}H$ and $A^{-1}$ are handled by functor
called `HbyA` and `Ainv`, respectively. These fields are generated by [RhieChowMassFlux.md] under the hood.
This means that we need to add the user object responsible for generating these fields:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-velocity-pressure.i block=UserObjects

As a last step, we add the SIMPLE executioner:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/2d-velocity-pressure.i block=Executioner

## Passive scalar advection

The `SIMPLE` executioner can be used to solve coupled problems involving both flow and passive scalar advection.
Advected passive scalars do not affect the flow distribution, and therefore can be solved after the velocity and
pressure fields have been computed using the `SIMPLE` algorithm.
Several systems may be used, for each passive scalar.

!syntax parameters /Executioner/SIMPLE

!syntax inputs /Executioner/SIMPLE

!syntax children /Executioner/SIMPLE
