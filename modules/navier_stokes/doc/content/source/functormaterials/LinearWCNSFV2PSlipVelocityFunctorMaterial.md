# LinearWCNSFV2PSlipVelocityFunctorMaterial

This material computes the slip velocity between a dispersed phase and the continuous phase for the
two-phase mixture model, using the linear finite volume discretization, and optionally the
diffusion velocity derived from it. It is the counterpart of
[WCNSFV2PSlipVelocityFunctorMaterial.md]; see that page for the closure and its references.

## The two relative velocities

Two distinct relative velocities appear in the mixture model and are easy to confuse.

The *slip*, or relative, velocity is the velocity of the dispersed phase with respect to the
continuous phase, $\bm{u}_{slip,d} = \bm{u}_d - \bm{u}_c$. It is what the algebraic closure
predicts, and it is declared under `slip_velocity_name`. It is what
[LinearWCNSFV2PMomentumDriftFlux.md] and [LinearWCNSFV2PEnergyDriftFlux.md] consume.

The *diffusion*, or drift, velocity is the velocity of the dispersed phase with respect to the
centre of mass of the mixture, $\bm{u}_{Md} = \bm{u}_d - \bm{u}_m$. It is what appears in the
conservation equations, and it is declared under `drift_velocity_name` when that parameter is
supplied. The two are related by the dispersed phase mass fraction $c_d = \alpha_d \rho_d /
\rho_m$,

\begin{equation}
\bm{u}_{Md} = \left( 1 - c_d \right) \bm{u}_{slip,d} \,,
\end{equation}

equation (28) of [!cite](manninen1996mixture). The phase transport equation must be advected at
$\bm{u}_m + \bm{u}_{Md} = \bm{u}_d$; advecting it at $\bm{u}_m + \bm{u}_{slip,d}$ instead is
the dilute approximation $c_d \to 0$. Computing the conversion requires the phase fraction, which
is why `fraction_dispersed` is needed whenever a drift velocity is requested.

## The density in the buoyancy factor

The closure is

\begin{equation}
\bm{u}_{slip,d} = \frac{\tau_d}{f_{drag}} \frac{\rho_d - \rho_m}{\rho_d} \bm{a} \,,
\qquad \tau_d = \frac{\rho_d d_d^2}{18 \mu_c} \,,
\qquad \bm{a} = \bm{g} + \frac{\bm{f}}{\rho_m}
  - \left( \bm{u}_m \cdot \nabla \right) \bm{u}_m
  - \frac{\partial \bm{u}_m}{\partial t} \,,
\end{equation}

equations (58) and (63) of [!cite](manninen1996mixture). The density subtracted from $\rho_d$ in
the buoyancy factor is the
*mixture* density, so the `rho` parameter of this object should be given `rho_mixture` and not the
continuous phase density. The two agree only in the dilute limit.

!alert warning
The nonlinear counterpart is given the continuous phase density by
[WCNSFVTwoPhaseMixturePhysics.md], so the two discretizations do not evaluate the same slip
velocity outside the dilute limit.

## The drag function

With `use_dispersed_phase_drag_model = true` this object evaluates a drag law itself rather than
reading one from a functor, and
[!param](/FunctorMaterials/LinearWCNSFV2PSlipVelocityFunctorMaterial/drag_model) selects which.
Three of the four options supply a drag function that closes the force balance; the fourth,
`ishii-zuber`, is a correlation for the relative velocity itself and is described in its own
section below. The rigid sphere law is taken first. The particle Reynolds number of the Schiller
and Naumann correlation is formed from the slip velocity and the continuous phase properties,

!equation
Re_p = \frac{\rho_c d_d |\bm{u}_{slip,d}|}{\mu_c}

which is its definition, see [!cite](manninen1996mixture) equation (39). That makes the drag depend
on the very quantity the closure determines. Writing $s = |\bm{u}_{slip,d}|$, $R = \rho_c d_d /
\mu_c$ and $s_0$ for the slip that the Stokes limit $f_{drag} \equiv 1$ would give, the closure and
the correlation together are the scalar equation

!equation
s \, f_{drag}(R s) = s_0

Since $f_{drag}$ is continuous, strictly increasing and at least unity, the left hand side rises
monotonically from zero, so the root is unique and bracketed by $[0, s_0]$. It is solved by a
Newton iteration safeguarded by that bracket, which converges in three or four steps from the
Stokes guess.

Solving it here is also what keeps the functor dependency graph acyclic. A drag functor evaluated
from the definition above depends on the slip velocity, so it cannot simultaneously be an input to
the slip velocity; that is why `use_dispersed_phase_drag_model` and a prescribed
`linear_coef_name` are mutually exclusive. Supply `linear_coef_name` only to impose a drag
function directly.

The `rho_c` and `mu_c` parameters are the continuous phase properties needed for $Re_p$, and are
required whenever the drag model is active. Note that they are distinct from `rho` and `mu`, which
are the *mixture* density and viscosity used by the buoyancy factor and the relaxation time.

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
[!cite](hibikiishii2002) equation (15), reproduced as an identity of the algebra rather than
approached to within a tolerance. The size independence is what the verification checks, by
instantiating the law at two diameters and requiring the same speed exactly.

`automatic` evaluates both single particle laws and takes whichever resists more, which selects the
regime by itself where it is not known in advance or varies across the domain.

## Verification

The closure and the slip to drift conversion are verified against their analytic values by the
`slip_closure_verification` test, which holds the velocity field at zero so that the acceleration
is exactly gravity and every resulting quantity is known in closed form.

## Relation to the nonlinear implementation

The two materials are deliberately kept separate rather than sharing a single object which inspects
the type of the velocity variable at run time. This material holds
[MooseLinearVariableFV.md] velocities directly, which has two consequences:

- The requirements of the closure, namely cell gradients and a time derivative, are properties of
  the velocity type rather than something rediscovered by a cast at each use. Cell gradients are
  requested unconditionally.
- The computation is performed in plain reals rather than automatic differentiation types, since
  the linear finite volume discretization assembles a matrix directly and has no use for the
  derivatives.

!alert note
Porosity is not treated. The linear finite volume discretization does not support a porous medium
treatment, so the velocities are always interstitial here. The nonlinear counterpart does accept a
porosity, which it uses to convert superficial velocities before forming the material derivative.

!syntax parameters /FunctorMaterials/LinearWCNSFV2PSlipVelocityFunctorMaterial

!syntax inputs /FunctorMaterials/LinearWCNSFV2PSlipVelocityFunctorMaterial

!syntax children /FunctorMaterials/LinearWCNSFV2PSlipVelocityFunctorMaterial
