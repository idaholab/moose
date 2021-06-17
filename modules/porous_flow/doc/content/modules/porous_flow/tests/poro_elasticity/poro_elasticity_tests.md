# Poroelasticity test descriptions

## Introduction

The PorousFlow module includes the ability to couple fluid flow to
solid mechanics, and thus includes poroelasticity, which is the theory
of a fully-saturated single-phase fluid with constant bulk density and
constant viscosity coupled to small-strain isotropic elasticity.

There is one important difference between the poroelasticity and most of PorousFlow, however.  The
time-derivative terms of poroelasticity are
\begin{equation}
\frac{1}{M}\dot{P} + \alpha\dot{\epsilon}_{ii} \ ,
\label{eqn.poro.dt}
\end{equation}
where $M$ is the Biot modulus:
\begin{equation}
\frac{1}{M} = \frac{(1-\alpha)(\alpha - \phi)}{K} +
\frac{\phi}{K_{\mathrm{f}}} \ ,
\label{biotmod.eqn}
\end{equation}
$P$ is the fluid porepressure, $\alpha$ is the Biot coefficient,
$\epsilon_{ii}$ is the volumetric strain, $\phi$ is the porosity, $K$
is the solid (drained) bulk modulus, and $K_{\mathrm{f}}$ is the fluid
bulk modulus.  Evidently from [biotmod.eqn], the Biot
modulus, $M$, should evolve with time as the porosity evolves.
Indeed, the terms in [eqn.poro.dt] are derived from the
continuity equation $\partial (\phi\rho)/\partial t +
\phi\rho\dot{\epsilon}_{ii}$ using the evolution of $\phi$ (and
$\rho = \rho_{0}\exp(P/K_{\mathrm{f}})$).  This derivation is discussed further [here](PorousFlowFullySaturatedMassTimeDerivative.md).  However, in the standard
analytical solutions of poroelasticity theory, the Biot modulus, $M$
is considered fixed.

The PorousFlow module allows porosity to vary with fluid porepressure
and volumetric strain, so usually the Biot modulus would vary too,
causing differences with the analytical solutions of poroelasticity.
Therefore, PorousFlow offers a porosity relationship that evolves
porosity in such a way as to keep $M$ fixed.  This is called [PorousFlowPorosityHMBiotModulus](PorousFlowPorosityHMBiotModulus.md).

PorousFlow is also built with finite strains in mind, whereas
poroelasticity is not.  Therefore, in comparisons with solutions from
poroelasticity theory, either the strain should be kept small, or the
various finite-strain switches in PorousFlow should be turned off
(they are all on by default).

## Simple tests

### Volumetric expansion due to increasing porepressure

The porepressure within a fully-saturated sample is increased:
\begin{equation}
P_{\mathrm{f}} = t \ .
\end{equation}
Zero mechanical pressure is applied to the sample's exterior, so that
no Neumann BCs are needed on the sample.  No fluid flow occurs since
the porepressure is increased uniformly throughout the sample.

The input file:

!listing modules/porous_flow/test/tests/poro_elasticity/vol_expansion.i

The effective stresses should then evolve as
$\sigma_{ij}^{\mathrm{eff}} = \alpha t \delta_{ij}$, and the
volumetric strain $\epsilon_{00}+\epsilon_{11}+\epsilon_{22} = \alpha
t/K$.  MOOSE produces this result correctly.

### Undrained oedometer test

A cubic single-element fully-saturated sample has roller BCs applied
to its sides and bottom.  All the sample's boundaries are impermeable.
A downwards (normal) displacement, $u_{z}$, is applied to its
top, and the rise in porepressure and effective stress is observed.
(Here $z$ denotes the direction normal to the top face.)  There is
no fluid flow in the single element.

!listing modules/porous_flow/test/tests/poro_elasticity/undrained_oedometer.i

Under these conditions, assuming constant porosity, and denoting the
height ($z$ length) of the sample by $L$:
\begin{equation}
\begin{array}{rcl}
P_{\mathrm{f}} & = & -K_{f}\log(1 - u_{z}) \ , \\
\sigma_{xx}^{\mathrm{eff}} & = & \left(K - \frac{2}{3}G\right)u_{z}/L \ ,  \\
\sigma_{zz}^{\mathrm{eff}} & = & \left(K + \frac{4}{3}G\right)u_{z}/L \ .
\end{array}
\end{equation}
MOOSE produces these results correctly.

### Porepressure generation of a confined sample

A single-element fully-saturated sample is constrained on all sides
and its boundaries are impermeable.  Fluid is pumped into the sample
via a source $s$ (kg.s$^{-1}$.m$^{-3}$) and the rise
in porepressure is observed.

The input file:

!listing modules/porous_flow/test/tests/poro_elasticity/pp_generation_fullysat_action.i

Denoting the strength of the source by $s$ (units are s$^{-1}$), the expected result is
\begin{equation}
\begin{array}{rcl}
\mathrm{fluid mass} & = & \mathrm{fluid\ mass}_{0} + st \ , \\
\sigma_{ij}^{\mathrm{eff}} & = & 0 \ ,  \\
P_{\mathrm{f}} & = & K_{\mathrm{f}}\log(M/(\phi\rho_{0})) \ , \\
\rho & = & \rho_{0}\exp(P_{\mathrm{f}}/K_{f}) \ ,\\
\phi & = & \alpha + (\phi_{0}-\alpha)\exp\left(
P_{\mathrm{f}}(\alpha - 1)/K\right) \ . \\
\end{array}
\end{equation}
Here $K$ is the solid bulk modulus.  MOOSE produces this result correctly.

### Porepressure generation of an unconfined sample

A single-element fully-saturated sample is constrained on all sides,
except its top.  All its boundaries are impermeable.  Fluid is pumped
into the sample via a source $s$ (kg.s$^{-1}$.m$^{-3}$) and the rise
in the top surface, the porepressure, and the stress are observed.

The input file:

!listing modules/porous_flow/test/tests/poro_elasticity/pp_generation_unconfined_basicthm.i

Regardless of the evolution of porosity, the following ratios result
\begin{equation}
\begin{array}{rcl}
\sigma_{xx}/\epsilon_{zz} & = & K - 2G/3 \ , \\
\sigma_{zz}/\epsilon_{zz} & = & K + 4G/3 \ ,  \\
P/\epsilon_{zz} & = & (K + 3G/3 + \alpha^{2}M)/\alpha - \alpha M \ .
\end{array}
\end{equation}
where $K$ is the undrained bulk modulus, $G$ the shear modulus,
$\alpha$ the Biot coefficient, and $M$ is the initial Biot modulus.
MOOSE produces these results correctly.

However, if the Biot modulus, $M$, is held fixed as the porosity
evolves, and the source is
\begin{equation}
s = S \rho_{0}\exp(P/K_{\mathrm{f}}) \ ,
\end{equation}
with $S$ being a *constant* volumetric source
(m$^{3}$.s$^{-1}$.m$^{-3}$) then
\begin{equation}
\begin{array}{rcl}
\epsilon_{zz} & = & \frac{\alpha M s t}{K + 4G/3 + \alpha^{2}M} \ , \\
P & = & M(st - \alpha\epsilon_{zz}) \ , \\
\sigma_{xx} & = & (K - 2G/3)\epsilon_{zz} \ , \\
\sigma_{zz} & = & (K + 4G/3)\epsilon_{zz} \ .
\end{array}
\end{equation}
The input file for this is:

!listing modules/porous_flow/test/tests/poro_elasticity/pp_generation_unconfined_constM_action.i

MOOSE produces these results correctly.

## Terzaghi consolidation of a drained medium

A saturated sample sits in a bath of water.  It is constrained on its sides and bottom.  Its sides and bottom are also impermeable.  Initially it is unstressed ($\sigma_{ij} = 0 = P_{\mathrm{f}}$, at $t=0$).  A normal stress, $q$, is applied to the sample's top.  The sample compresses instantaneously due to the instantaneous application of $q$, and then slowly compresses further as water is squeezed out from the sample's top surface.

This is a classic test.  See, for example, Section 2.2 of the online manuscript: Arnold Verruijt "Theory and Problems of Poroelasticity" Delft University of Technology 2013.  But note that the "sigma" in that paper is the negative of the stress in PorousFlow.  Denote the sample's height ($z$-length) by $h$.  Define
\begin{equation}
p_{0} = \frac{\alpha q M}{S(K + 4G/3) + \alpha^{2}M} \ .
\end{equation}
This is the porepressure that results from the instantaneous application of $q$: MOOSE calculates this correctly.  The solution for porepressure is
\begin{equation}
P_{\mathrm{f}} = \frac{4p_{0}}{\pi}\sum_{n=1}^{\infty} \frac{(-1)^{n-1}}{2n-1} \cos \left( \frac{(2n-1)\pi z}{2h} \right) \exp \left( -(2n-1)^{2} \pi^{2} \frac{ct}{4h^{2}} \right) \ .
\end{equation}
In this equation, $c$ is the "consolidation coefficient": $c = k (K + 4G/3) M/(K + 4G/3 + \alpha^{2} M)$, where the permeability tensor is $k_{ij} = \mathrm{diag}(k, k, k)$.  The so-called degree-of-consolidation is defined by
\begin{equation}
U = \frac{u_{z} - u_{z}^{0}}{u_{z}^{\infty} - u_{z}^{0}} \ ,
\end{equation}
where $u_{z}$ is the vertical displacement of the top surface (downwards is positive), and $u_{z}^{0}$ is the instantaneous displacement due to the instantaneous application of $q$, and $u_{z}^{\infty}$ is the final displacement.  This has solution
\begin{equation}
U = 1 - \frac{8}{\pi^{2}}\sum_{n=1}^{\infty} \frac{1}{(2n-1)^{2}} \exp \left(-(2n-1)^{2}\pi^{2}\frac{ct}{4h^{2}} \right) \ .
\end{equation}

There are a few different variants of this test in the test suite, that use the PorousFlow action system, constant Biot modulus, constant porosity, the fully-saturated version of PorousFlow, etc.  You are encouraged to explore these to get a feeling for how the differences impact the output.  The input file for that matches the theoretical setup exactly is:

!listing modules/porous_flow/test/tests/poro_elasticity/terzaghi_constM.i

MOOSE produces the expected results correctly, as may be seen from [terzaghi_u] and [terzaghi_p].

!media poro_elasticity/terzaghi_u.png style=width:50%;margin-left:10px caption=Degree of consolidation in the Terzaghi experiment.  id=terzaghi_u

!media poro_elasticity/terzaghi_p.png style=width:50%;margin-left:10px caption=Porepressure at various times in the Terzaghi experiment.  id=terzaghi_p





## Mandel's consolidation of a drained medium

A sample's dimensions are $-a \leq x \leq a$ and $-b \leq y \leq b$,
and it is in plane strain (no $z$ displacement).  It is squashed with
constant normal force by impermeable, frictionless plattens on its top
and bottom surfaces (at $y = \pm b$).  Fluid is allowed to leak out
from its sides (at $x = \pm a$), but all other surfaces are
impermeable.  This is called Mandel's problem and it is shown
graphically in [mandel_setup.fig]

!media poro_elasticity/mandel_setup.png style=width:50%;margin-left:10px caption=The setup of the Mandel experiment: a force squashes a porous material with impermeable plattens.  This causes fluid to seep from the material. id=mandel_setup.fig

The interesting feature of this problem (apart from that it can be
solved analytically) is that the porepressure in the sample's center
increases for a short time after application of the force.
This is because the leakage of the fluid from the sample's sides
causes an apparent softening of the material near those sides.  This
means stress concentrates towards the sample's center which causes an
increase in porepressure.  Of course, eventually the fluid totally
drains from the sample, and the porepressure is zero.  As the fluid
drains from the sample's sides the plattens move slowly towards each
other.

The solution for porepressure and displacements is given in [!citet](doi:10.1002/nag.1610120508).  The solution involves
rather lengthy infinite series, so I will not write it here.

As is common in the literature, this is simulated by considering the
quarter-sample, $0\leq x \leq a$ and $0\leq y\leq b$, with
impermeable, roller BCs at $x=0$ and $y=0$ and $y=b$.  Porepressure is
fixed at zero on $x=a$.  Porepressure and displacement are initialised
to zero.  Then the top ($y=b$) is moved downwards with prescribed
velocity, so that the total force that is induced by this downwards
velocity is fixed.  The velocity is worked out by solving Mandel's
problem analytically, and the total force is monitored in the
simulation to check that it indeed remains constant.

The simulations in the PorousFlow test suite use 10 elements in the
$x$ direction and 1 in the $y$ direction.  Four types of simulation
are run:

1. HM.  This uses standard PorousFlow Materials and Kernels, in particular it uses the "HM" porosity relationship.  This is not expected to agree perfectly with the analytical solutions because the solutions assume constant Biot modulus.

!listing modules/porous_flow/test/tests/poro_elasticity/mandel.i

2. constM.  This is identical to the HM case, save that it uses a porosity evolution law that keeps the Biot modulus fixed.  It is therefore expected to agree with the analytical solutions.

!listing modules/porous_flow/test/tests/poro_elasticity/mandel_constM.i

3. FullSat.  This uses the FullySaturated versions of the fluid mass time derivative and the fluid flux.  In this case the Biot modulus is kept fixed, so it is expected to agree with the analytical solutions.

!listing modules/porous_flow/test/tests/poro_elasticity/mandel_fully_saturated.i

4. FullSatVol.  This uses the FullySaturated versions of the fluid mass time derivative and the fluid flux, and does not multiply by the fluid density.  Therefore this version is identical to what is usually implemented in poro-elastic codes.  It is linear and therefore converges in only one iteration.  In this case the Biot modulus is kept fixed, so it is expected to agree with the analytical solutions.

!listing modules/porous_flow/test/tests/poro_elasticity/mandel_basicthm.i

Of course there are minor discrepancies between the last three and the
analytical solution that are brought about through spatial and
temporal discretisation errors.  The figures below present the
results.

!media poro_elasticity/mandel_ver_disp.png style=width:50%;margin-left:10px caption=The vertical displacement of the platten as a function of time. id=mandel_ver_disp_fig

!media poro_elasticity/mandel_hor_disp.png style=width:50%;margin-left:10px caption=The horizontal displacement of the platten's top-right corner. id=mandel_hor_disp_fig

!media poro_elasticity/mandel_HM.png style=width:50%;margin-left:10px caption=The porepressure at various points in the sample in the HM model ("a" is equal to unity). id=mandel_HM

!media poro_elasticity/mandel_FSV.png style=width:50%;margin-left:10px caption=The porepressure at various points in the sample in the FullSatVol model ("a" is equal to unity). id=mandel_FSV

!media poro_elasticity/mandel_force.png style=width:50%;margin-left:10px caption=The total downwards force on the platten as a function of time.  This should be unity. id=mandel_force

!bibtex bibliography
