# Heat advection in a 1D bar

Consider the case of a single-phase fluid in 1 dimension, $0\leq x
\leq 1$, with the porepressure fixed at the boundaries:
\begin{equation}
P(x=0, t) = 1 \ \ \ \mathrm{and}\ \ \ P(x=1, t) = 0 \ .
\end{equation}
With zero gravity, and high fluid bulk modulus, the Darcy equation
implies that the solution is $P(x, t) = 1 - x$, with
\begin{equation}
v = k/\mu
\end{equation}
being the constant "Darcy
velocity" from $x=0$ to $x=1$.  (The velocity of the individual
  fluid particles, this is divided by the porosity.).  Here $k$ is the porous medium's
permeability, and $\mu$ is the fluid dynamic viscosity.

Suppose that the fluid internal energy is given by $CT$, where $C$ is
the specific heat capacity and $T$ is its temperature.  Assuming that
$P/\rho \ll CT$, then the fluid's enthalpy is also $CT$.  In this
case, the energy equation reads
\begin{equation}
\left((1 - \phi)\rho_{R}C_{R} + \phi\rho C \right) \frac{\partial
  T}{\partial t} + C \rho v \frac{\partial T}{\partial x} = 0 \ .
\end{equation}
This is the wave equation with velocity
\begin{equation}
v_{T} = \frac{C\rho v}{(1 - \phi)\rho_{R}C_{R} + \phi\rho C} \ .
\end{equation}
Recall that the "Darcy velocity" is $v=k/\mu$.

Let the initial condition for $T$ be $T(x, t=0) = 200$.  Apply the
boundary conditions
\begin{equation}
T(x=0, t) = 300 \ \ \ \mathrm{and} \ \ \ T(x=1, t) = 200 \ .
\end{equation}
At $t=0$ this creates a front at $x=0$.  Choose the parameters $C=2$,
$C_{R}=1$, $\rho=1000$, $\rho_{R}=125$, $\phi=0.2$, $k=1.1$, $\mu=4.4$
(all in consistent units), so that $v_{T}=1$ is the front's velocity.

The input file used in this page is:

!listing modules/porous_flow/test/tests/heat_advection/heat_advection_1d_fully_saturated_action.i

The sharp front is *not* maintained by
MOOSE.  This is due to numerical diffusion, whose magnitude depends on the stabilization scheme used.  This is explained in great detail in the pages on [numerical stabilization](stabilization.md).  Nevertheless, MOOSE advects the smooth front with the correct
velocity, as shown in [heat_advection_1d.fig].

[heat_advection_1d.fig] shows three versions of the advection, which are activated by setting the `stabilization` parameter in the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) Action:

- with no stabilization

```
  stabilization = none
```

- with full upwinding

```
  stabilization = Full
```

- with KT stabilization

```
  stabilization = KT
```

With no numerical stabilization, two additional points may also be
noticed: (1) the lack of upwinding has produced a "bump" in the
temperature profile near the hotter side; (2) the lack of upwinding
means the temperature profile moves slightly slower than it should.
These two affects reduce as the mesh density is increased, however.


!media porous_flow/tests/heat_advection/heat_advection.png style=width:90%;margin-left:10px caption=Results of heat advection via a fluid in 1D.  The fluid flows with constant Darcy velocity of 0.25m/s to the right, and this advects a temperature front at velocity 1m/s to the right.  The pictures above that the numerical implementation of PorousFlow (including upwinding) diffuses sharp fronts, but advects them at the correct velocity (notice the centre of the upwinded front is at the correct position in each picture).  Less diffusion is experienced without upwinding or with KT stabilization.  Left: temperature 0.1s.  Right: temperature at 0.6s. id=heat_advection_1d.fig




