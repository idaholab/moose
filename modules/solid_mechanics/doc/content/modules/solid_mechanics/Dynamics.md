# Dynamics

Dynamic problems -- the transient response of a structure to time-varying loads, wave propagation, and
similar -- extend the [balance of linear momentum](BalanceOfLinearMomentum.md) with an inertial term.

## Equation of Motion id=equation

The strong form of the dynamic balance of linear momentum is
\begin{equation}
   \rho\, \ddot{u}_i + \eta\, \rho\, \dot{u}_i = \sigma_{ij,j} + b_i
\end{equation}
where $\rho$ is the mass density, $\ddot{u}_i$ the acceleration, $\dot{u}_i$ the velocity, $\eta$ the
mass-proportional damping coefficient, $\sigma_{ij}$ the stress, and $b_i$ the body force per unit
volume.  This is the [static balance](BalanceOfLinearMomentum.md) with the inertial force $\rho\ddot
u_i$ and a mass-proportional damping force $\eta\rho\dot u_i$ added; it reduces to the static problem
when $\dot u_i = \ddot u_i = 0$, and it holds on the reference or current configuration following the
same total/updated convention as the static case.

Each term is supplied by a separate object, one per displacement component:

| Term                    | Meaning                   | Object                                                                                 |
| ----------------------- | ------------------------- | -------------------------------------------------------------------------------------- |
| $\rho\,\ddot{u}_i$      | inertia                   | [`InertialForce`](InertialForce.md)                                                    |
| $\eta\,\rho\,\dot{u}_i$ | mass-proportional damping | [`InertialForce`](InertialForce.md) (`eta`)                                            |
| $\sigma_{ij,j}$         | internal force            | Lagrangian [stress-divergence kernel](BalanceOfLinearMomentum.md) + constitutive model |
| $b_i$                   | body force                | e.g. [`Gravity`](Gravity.md)                                                           |

Stiffness-proportional damping does not appear as a separate term: it is carried inside $\sigma_{ij}$
as a [viscous stress](#damping).

## Time Integration id=time-integration

The velocity and acceleration are related to the displacement by a time-integration scheme.  The
implicit Newmark [!citep](newmark1959amethod) and Hilber-Hughes-Taylor (HHT) [!citep](hughes2000fem)
schemes are described below, along with experimental support for explicit dynamics.

### Newmark

Newmark integration expresses the acceleration and velocity at $t+\Delta t$ in terms of the state at
$t$ and the displacement at $t+\Delta t$:
\begin{equation}
\begin{aligned}
\ddot{u}_i(t+\Delta t) &= \frac{u_i(t+\Delta t)-u_i(t)}{\beta \Delta t^2}
   - \frac{\dot{u}_i(t)}{\beta \Delta t} + \frac{\beta - \tfrac{1}{2}}{\beta}\,\ddot{u}_i(t), \\
\dot{u}_i(t+\Delta t) &= \dot{u}_i(t) + (1-\gamma)\Delta t\, \ddot{u}_i(t)
   + \gamma \Delta t\, \ddot{u}_i(t+\Delta t),
\end{aligned}
\end{equation}
with parameters $\beta$ and $\gamma$.  Common choices:

- $\beta = \tfrac{1}{4}$, $\gamma = \tfrac{1}{2}$: constant average acceleration -- unconditionally
  stable with no numerical damping (recommended with a constant time step).
- $\beta = \tfrac{1}{6}$, $\gamma = \tfrac{1}{2}$: linear acceleration.
- $\beta = 0$, $\gamma = \tfrac{1}{2}$: the central-difference method in theory (but see
  [Explicit Dynamics](#explicit-dynamics)).

!alert note
For $\gamma = \tfrac{1}{2}$ the Newmark method is second-order accurate and, for
$\tfrac{1}{2} \le \gamma \le 2\beta$, unconditionally stable; it is first-order accurate for other
values of $\gamma$.

### Hilber-Hughes-Taylor

The constant-average-acceleration Newmark scheme introduces no numerical damping, which can leave
spurious high-frequency content.  HHT damps the high frequencies by evaluating the non-inertial terms
at a weighted time using the parameter $\alpha$:
\begin{equation}
   \rho\,\ddot{u}_i(t+\Delta t)
   + \eta\rho\left[(1+\alpha)\dot{u}_i(t+\Delta t) - \alpha\,\dot{u}_i(t)\right]
   = (1+\alpha)\left[\sigma_{ij,j} + b_i\right]_{t+\Delta t}
   - \alpha\left[\sigma_{ij,j} + b_i\right]_{t}.
\end{equation}
Choosing $-\tfrac{1}{3} \le \alpha \le 0$ with $\beta = \tfrac{(1-\alpha)^2}{4}$ and
$\gamma = \tfrac{1}{2} - \alpha$ keeps the scheme second-order accurate and unconditionally stable
while damping frequencies above $\tfrac{1}{2\Delta t}$.  The inertial term is not weighted; the
internal force, damping, and external loads are, so loads such as gravity and pressure are evaluated
at $t + (1+\alpha)\Delta t$.  Setting $\alpha = 0$ recovers Newmark.

### Explicit Dynamics id=explicit-dynamics

!alert warning title=Experimental
Explicit dynamics support is under active development and should be considered experimental.

The Newmark and HHT schemes above are implicit.  For wave-propagation and high strain-rate problems,
an explicit central-difference scheme -- which advances the solution without a nonlinear solve, at the
cost of a conditionally stable (critical) time step -- is available through the
[`ExplicitMixedOrder`](ExplicitMixedOrder.md) time integrator.

!alert note title=Newmark beta = 0 is not an explicit solve
Setting $\beta = 0$ in the Newmark integrator is the central-difference method *in theory*, but the
implementation still assembles and solves the full system every step -- it is not a genuine explicit
solve.  For explicit dynamics, use [`ExplicitMixedOrder`](ExplicitMixedOrder.md).

## Damping id=damping

Rayleigh damping combines a mass-proportional and a stiffness-proportional contribution, giving a
damping ratio that varies with the modal frequency $\omega$ as
\begin{equation}
   \xi(\omega) = \frac{\eta}{2\omega} + \frac{\zeta}{2}\,\omega,
\end{equation}
where $\eta$ is the mass-proportional and $\zeta$ the stiffness-proportional coefficient.
To obtain a target damping ratio $\xi_t$ at two frequencies $\omega_1$ and $\omega_2$, solving
$\xi(\omega_1) = \xi(\omega_2) = \xi_t$ gives
\begin{equation}
   \eta = 2\,\xi_t\,\frac{\omega_1 \omega_2}{\omega_1 + \omega_2}, \qquad
   \zeta = \frac{2\,\xi_t}{\omega_1 + \omega_2}.
\end{equation}
Between $\omega_1$ and $\omega_2$ the actual ratio dips slightly below $\xi_t$; outside the band it
rises.

The mass-proportional force $\eta\rho\dot{u}_i$ is applied by the [`InertialForce`](InertialForce.md)
kernel through its `eta` parameter. The stiffness-proportional contribution is treated as a
*viscous stress* and can be written as a rate-dependent constitutive model.

## Implementation and Usage id=usage

A dynamic model adds, for each displacement component:

- a Lagrangian [stress-divergence kernel](BalanceOfLinearMomentum.md) for the internal force,
- an [`InertialForce`](InertialForce.md) kernel for $\rho\ddot{u}_i$ (and, via `eta`, the
  mass-proportional damping), and
- a time integrator, with the Newmark `beta`/`gamma` and, for HHT, the `alpha` parameter.

The [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action sets up the
stress-divergence kernels and the strain calculator; the inertial kernel and time integrator are added
alongside it.  A material `density` is required, and stiffness-proportional damping, when needed, is
supplied as the [viscous constitutive stress](#damping) above.

!alert note
For dynamic problems, prescribe boundary motion with [`PresetDisplacement`](PresetDisplacement.md) and
[`PresetAcceleration`](PresetAcceleration.md) rather than a plain Dirichlet condition on the
displacement.

## Static Initialization id=static-initialization

To start from a statically loaded state (for example under gravity), run an initial static step with
the inertial contribution disabled, then enable it for the transient.  Activating the
[`InertialForce`](InertialForce.md) kernel only after the first step (for instance with a
[`Controls`](/Controls/index.md) block) makes the first solve return the static equilibrium under the
sustained load.

!bibtex bibliography
