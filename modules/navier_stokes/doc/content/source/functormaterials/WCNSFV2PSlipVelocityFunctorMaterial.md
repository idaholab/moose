# WCNSFV2PSlipVelocityFunctorMaterial

This material computes the slip velocity between a dispersed phase and the continuous phase for the
two-phase mixture model, and optionally the diffusion velocity derived from it. It evaluates the
closure in plain reals, for the velocities of the linear finite volume discretization.
[ADWCNSFV2PSlipVelocityFunctorMaterial.md] evaluates the same closure, from the same templated
implementation, with automatic differentiation, for the nonlinear finite volume discretization.

## The two relative velocities

Two distinct relative velocities appear in the mixture model and are easy to confuse.

The *slip*, or relative, velocity is the velocity of the dispersed phase with respect to the
continuous phase, $\bm{u}_{slip,d} = \bm{u}_d - \bm{u}_c$. It is what the algebraic closure
predicts, and it is declared under
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/slip_velocity_name). It is what
[WCNSFV2PMomentumDriftFlux.md], [LinearWCNSFV2PMomentumDriftFlux.md] and
[LinearWCNSFV2PEnergyDriftFlux.md] consume, the diffusion stress being a sum over both phases that
collapses onto the slip velocity for a single dispersed phase.

The *diffusion*, or drift, velocity is the velocity of the dispersed phase with respect to the
centre of mass of the mixture, $\bm{u}_{Md} = \bm{u}_d - \bm{u}_m$. It is what appears in the
conservation equations, and it is declared under
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/drift_velocity_name) when that
parameter is supplied. The two are related by the dispersed phase mass fraction
$c_d = \alpha_d \rho_d / \rho_m$ [!cite](manninen1996mixture),

\begin{equation}
\bm{u}_{Md} = \left( 1 - c_d \right) \bm{u}_{slip,d} \,.
\end{equation}

The phase transport equation must be advected at
$\bm{u}_m + \bm{u}_{Md} = \bm{u}_d$, so [LinearWCNSFV2PScalarAdvection.md] and
[INSFVScalarFieldAdvection.md] take the drift velocity and not the slip velocity; advecting it at $\bm{u}_m + \bm{u}_{slip,d}$ instead is
the dilute approximation $c_d \to 0$. Computing the conversion requires the phase fraction, which
is why [!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/fraction_dispersed) is
needed whenever a drift velocity is requested.

## The density in the buoyancy factor

The closure is

\begin{equation}
\bm{u}_{slip,d} = \frac{\tau_d}{f_{drag}} \frac{\rho_d - \rho_m}{\rho_d} \bm{a} \,,
\qquad \tau_d = \frac{\rho_d d_d^2}{18 \mu_c} \,,
\qquad \bm{a} = \bm{g} + \frac{\bm{f}}{\rho_m}
  - \left( \bm{u}_m \cdot \nabla \right) \bm{u}_m
  - \frac{\partial \bm{u}_m}{\partial t} \,,
\end{equation}

where $\tau_d$ is the particle relaxation time [!cite](bilicki1990dragmodel), $f_{drag}$ the drag
function, $d_d$ the particle diameter, $\bm{g}$ the acceleration of gravity, $\bm{f}$ the volumetric
force and $\bm{u}_m$ the mixture velocity. This is the algebraic slip closure of
[!cite](manninen1996mixture). The density subtracted from $\rho_d$ in the buoyancy factor
is the *mixture* density, so [!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/rho)
should be given `rho_mixture` and not the continuous phase density. The two agree only in the dilute
limit. The viscosity of the relaxation time is that of the continuous phase, supplied under
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/mu).

## The drag function

With [!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/use_dispersed_phase_drag_model)
set to `true` this object evaluates a drag law itself rather than reading one from a functor, and
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/drag_model) selects which.
Three of the four options supply a drag function that closes the force balance; the fourth,
`ishii-zuber`, is the multi-bubble correlation of [!cite](ishiizuber1979drag) in the form given by
[!cite](hibikiishii2003driftflux), which predicts the relative velocity itself. The rigid sphere
law is taken first. The particle Reynolds number of the Schiller and Naumann correlation is formed
from the slip velocity and the continuous phase properties,

!equation
Re_p = \frac{\rho_c d_d |\bm{u}_{slip,d}|}{\mu_c}

which is its definition [!cite](manninen1996mixture). That makes the drag depend
on the very quantity the closure determines. Writing $s = |\bm{u}_{slip,d}|$, $R = \rho_c d_d /
\mu_c$ and $s_0$ for the slip that the Stokes limit $f_{drag} \equiv 1$ would give, the closure and
the correlation together are the scalar equation

!equation
s \, f_{drag}(R s) = s_0

Since $f_{drag}$ is continuous, strictly increasing and at least unity, the left hand side rises
monotonically from zero, so the root is unique and lies in $[0, s_0]$. It is solved by Newton's
method, which converges in a few steps from the Stokes guess: the left hand side is also convex, so
no Newton step leaves that interval.

Solving it here is also what keeps the functor dependency graph acyclic. A drag functor evaluated
from the definition above depends on the slip velocity, so it cannot simultaneously be an input to
the slip velocity; that is why
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/use_dispersed_phase_drag_model)
and a prescribed [!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/linear_coef_name)
are mutually exclusive. Supply
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/linear_coef_name) only to
impose a drag function directly.

[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/rho_c) is the continuous phase
density needed for $Re_p$, and is required whenever the drag model is active. It is distinct from
[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/rho), the *mixture* density of
the buoyancy factor. The viscosity in $Re_p$ is the continuous phase viscosity already supplied
under [!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/mu).

### The distorted particle law

Above roughly a millimetre a bubble no longer behaves as a rigid sphere: it deforms, and its drag
coefficient grows with size rather than falling with Reynolds number,

!equation
C_D = \frac{2}{3} d_d \sqrt{\frac{g \Delta\rho}{\sigma}}

Manninen's closure carries the drag as the linear function $f_{drag} = C_D Re_p / 24$, which
normalises Stokes drag to unity, so that coefficient becomes

!equation
f_{drag} = \frac{d_d^2 \rho_c \left|\bm{u}_{slip,d}\right|}{36 \mu_c}
           \sqrt{\frac{g \Delta\rho}{\sigma}}

This is linear in the slip speed, so substituting it into the closure turns the force balance from
linear into quadratic in that speed and the root is explicit: no iteration is needed on this branch.
Evaluating it gives the terminal velocity

!equation
s_\infty = \sqrt{2}\left(\frac{g \sigma \Delta\rho}{\rho_c^2}\right)^{1/4}

independent of the particle diameter, which cancels between the relaxation time and the drag
coefficient. That is Ishii's drift velocity correlation for the bubbly regime, see
[!cite](hibikiishii2002), reproduced as an identity of the algebra rather than
approached to within a tolerance.

`automatic` evaluates both single particle laws and takes whichever resists more, which selects the
regime by itself where it is not known in advance or varies across the domain.

## The two versions

Both versions evaluate the closure from one templated implementation, and they differ only in the
velocities they hold and the type they compute in:

- This material holds [MooseLinearVariableFV.md] velocities directly, requests their cell gradients
  unconditionally, and computes in plain reals, since the linear finite volume discretization
  assembles a matrix directly and has no use for the derivatives.
- [ADWCNSFV2PSlipVelocityFunctorMaterial.md] accepts either velocity type, takes the velocity
  gradients and time derivatives from the functor interface, and computes with automatic
  differentiation so that the slip velocity carries its derivatives into the Jacobian of a
  nonlinear solve.

!alert note
Porosity is not treated: the velocities are taken to be interstitial.

!syntax parameters /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

!syntax inputs /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

!syntax children /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial
