# Porosity

Porosity may be fixed at a constant value, or it may be a function of the
effective porepressure, the volumetric strain, the temperature and/or chemical precipitates

Available porosity formulations include:

## Constant: [PorousFlowPorosityConst](/PorousFlowPorosityConst.md)

The simplest case where porosity does not change during the simulation. A single value of porosity
can be used, or a spatially varying porosity `AuxVariable` can be used to define a heterogeneous
porosity distribution.

## Linear: [PorousFlowPorosityLinear](/PorousFlowPorosityLinear.md)

In this case

\begin{equation}
\label{eq:poro_evolve_linear}
\phi = \phi_{\mathrm{ref}} + A(P_{f} - P_{f\ \mathrm{ref}}) + B(T - T_{\mathrm{ref}}) + C(\epsilon^{\mathrm{total}}_{ii} - \epsilon^{\mathrm{total}}_{ii\ \mathrm{ref}}) \ .
\end{equation}

Here the "ref" values are reference values, $P_{f}$ is the [effective fluid pressure](PorousFlowEffectiveFluidPressure.md), $T$ is the [temperature](PorousFlowTemperature.md) and $\epsilon^{\mathrm{total}}_{ii}$ is the [total volumetric strain](PorousFlowVolumetricStrain.md).  These may be spatially-varying `AuxVariables` to define a heterogeneous porosity distribution.  $A$, $B$ and $C$ are real-valued coefficients.  A lower-bound on porosity, $\phi \geq \phi_{\mathrm{min}}$, may also be defined.

## Exponential: [PorousFlowPorosity](/PorousFlowPorosity.md)

Using [PorousFlowPorosity](/PorousFlowPorosity.md) with appropriately set flags,
porosity can depend on:

- total strain, with `mechanical = true`
- effective porepressure, with `fluid = true`
- temperature, with `thermal = true`
- precipitated minerals, with `chemical = true`

A combination of these may be used, to simulate, for instance, THM or HM coupling.

## Theoretical evolution of porosity leading to the exponential form

The evolution of the porosity is governed by [!citep](detournayET93, chen2009)
\begin{equation}
\label{eq:phi_dog}
\frac{\partial}{\partial t}(\phi + M) = (\alpha_{B} -
(\phi + M))\frac{\partial}{\partial t}
\left(\epsilon^{\mathrm{total}}_{ii} - \alpha_{T} T +
\frac{1-\alpha_{B}}{K}
P_{\mathrm{f}} \right) \ .
\end{equation}
Here $K$ is the bulk modulus of the drained porous skeleton: $1/K
= \delta_{ij}\delta_{kl}C_{ijkl}$.  Also $\alpha_{T}$ is the volumetric coefficient of thermal expansion.  $M$ denotes the total precipitated mineral concentration:
\begin{equation}
M = \sum_{\mathrm{minerals}}w_{\mathrm{mineral}} C_{\mathrm{mineral}} \ ,
\end{equation}
where $w$ is the user-defined weight for each mineral and $C$ is the concentration (m$^{3}$/m$^{3}$)
of precipitated mineral.

#### Special form for fracture flow

It is sometimes convenient to extend [eq:phi_dog] to include a new Biot coefficient, $\alpha_{B}'$:
\begin{equation}
\label{eq:phi_dog_prime}
\frac{\partial}{\partial t}(\phi + M) = (\alpha_{B} -
(\phi + M))\frac{\partial}{\partial t}
\left(\epsilon^{\mathrm{total}}_{ii} - \alpha_{T} T +
\frac{1-\alpha_{B}'}{K}
P_{\mathrm{f}} \right) \ .
\end{equation}
This form is useful when modelling flow through fractures, where the porosity *increases* with porepressure, which can be simulated with $\alpha_{B}'>1$.

#### Solution

The equation for porosity has solution
\begin{equation}
\label{eq:poro_evolve}
\phi + M = \alpha_{B} + (\phi_{0} + M_{\mathrm{ref}} - \alpha_{B})\times \exp \left( \frac{\alpha_{B}'
  - 1}{K}(P_{f} - P_{f}^{\mathrm{ref}}) - \epsilon^{\mathrm{total}}_{ii} + \alpha_{T}(T - T^{\mathrm{ref}}) \right) \ ,
\end{equation}
where $\phi_{0}$ is the porosity at reference porepressure, zero elastic strain, reference
temperature and reference mineral concentration.  Note this porosity can become negative, and an
option for ensuring positivity is detailed below.

#### Time-lag in mineralisation problems

With mineralisation, $\phi$ now depends on total mineral concentration, $M$.  However, the evolution
of $M$ is governed by $\dot{M} = \phi S_{\mathrm{aq}} I$, where $I$ is a reaction rate which is independent of
porosity (but dependent on the primary chemical species, temperature, etc) and $S_{\mathrm{aq}}$ is the aqueous phase saturation.  Therefore a circular
dependency exists: $\phi$ depends on $M$, and $M$ depends on $\phi$.  This could be broken by
promoting porosity to a MOOSE Variable, and solving for it.  Instead, PorousFlow replaces $M$ in
[eq:poro_evolve] by the approximate form
\begin{equation}
M \rightarrow M_{\mathrm{old}} + \phi_{\mathrm{old}} S_{\mathrm{aq}} I \mathrm{d}t \ .
\end{equation}
Note that the *old* value of porosity is used on the right-hand-side, which breaks the cyclic dependency problem.

#### Thermal-mechanical simulations

Without porepressure and mineralisation effects, the correct expression for porosity as a
function of volumetric strain and temperature is
\begin{equation}
\label{eq:poroTM_evolve}
\phi = 1 + (\phi_{0} - 1)\times \exp \left(-
\epsilon^{\mathrm{total}}_{ii} + \alpha_{T}(T - T^{\mathrm{ref}}) \right) \ .
\end{equation}

These expressions may be modified to include the effects of plasticity.

#### Preventing negative porosity

The exponential expressions [eq:poro_evolve] and [eq:poroTM_evolve] can yield
negative porosity values, which are unphysical.  To ensure positivity
of $\phi$, PorousFlow offers the following option.  First write both
equations in the form
\begin{equation}
\phi = A + (B - A) \exp(x) \ .
\end{equation}
In deriving specific forms for $A$, $B$ and $x$, above, it has been assumed that $x$ is small.  Since
physically $1\geq A > B > 0$, for $x<0$ the porosity will be physically meaningful, but for $x\gg 0$,
$\phi$ can become negative.  For example, for positive volumetric strain ($x<0$) the porosity is
always physically meaningful.  However, when the porous material is squashed with negative and large
volumetric strain ($x\gg 0$) the porosity can become negative.

Define
\begin{equation}
C = \log(A/(A-B)) \ .
\end{equation}
Then, for $x>0$, the above expression for $\phi$ is replaced by
\begin{equation}
\phi = A + (B - A) \exp\left(C (1 - \exp(-x/C)) \right)  \geq 0 \ .
\end{equation}
At first glance this expression may appear rather obscure.  It has been constructed to satisfy the
following requirements:

- As $x\rightarrow 0$, $\phi\rightarrow B$, which is necessary for
  continuity at $x=0$.
- As $x\rightarrow\infty$, $\phi\rightarrow 0$, which is
  physically desireable.
- The expression is monotonically decreasing, which is both
  physically desirable and computationally desirable (otherwise there
  may be non-unique solutions in a PorousFlow simulation).
- Finally, at $x=0$,
  its derivative is $\phi' = B - A$, as desired from continuity of the
  derivative at $x=0$.

## Evolution of porosity for an isothermal, mineral-free, situation

The evolution of porosity is fundamental to the coupling between flow and solid mechanics.  Consider
the isothermal situation with no plasticity.  The following presentation is mostly drawn from [!citep](detournayET93).

Denote the change of a quantity, q, by $\Delta
q$.  Recall that the porosity is defined by $\phi = V_{\mathrm{f}}/V$,
where $V$ is an arbitrary volume of the porous material, and
$V_{\mathrm{f}}$ is the porevolume within that volume.  Also, by
definition of the effective stress,
\begin{equation}
\Delta \epsilon_{ij} = C_{ijkl}(\Delta\sigma_{ij}^{\mathrm{tot}}  + \alpha_{B}
\delta_{ij}\Delta P_{\mathrm{f}})
\ .
\end{equation}
Taking the trace of this equation, and using $V^{-1}\Delta V = \Delta
\epsilon_{ii}$ yields
\begin{equation}
\frac{\Delta V}{V} = \delta_{ij}C_{ijkl}(\Delta\sigma_{ij}^{\mathrm{tot}}+
\alpha_{B} \delta_{ij}\Delta P_{\mathrm{f}})
\ .
\end{equation}
In most instances it is appropriate to write this equation as
\begin{equation}
\label{eq:fund_volstrain}
\frac{\Delta V}{V} = -\frac{1}{K}(P_{\mathrm{mech}} - \alpha_{B} \delta_{ij}\Delta P_{\mathrm{f}})
\ .
\end{equation}
(which is Detournay and Cheng Eqn$~$(20a)) where the total mechanical pressure is
\begin{equation}
P_{\mathrm{mech}} = - \textrm{Tr}\sigma/3 \ .
\end{equation}
and $K$ is the so-called *drained* bulk modulus $K^{-1} = \delta_{ij}C_{ijkl}\delta_{kl}$.
To find the evolution equation for porosity, a similar equation for
$\Delta V_{\mathrm{f}}/V_{\mathrm{f}}$ must be derived.

Assuming linearity
\begin{equation}
\label{eq:deltavf}
\frac{\Delta V_{\mathrm{f}}}{V_{\mathrm{f}}} = A_{ij}
(\Delta\sigma_{ij}^{\mathrm{tot}} + B \delta_{ij}\Delta
P_{\mathrm{f}}) \ .
\end{equation}
(This is Detournay and Cheng Eqn$~$(20b).)  The Betti-Maxwell reciprocal theorem yields $A_{ij}$ and $B$, as is
now shown.

The work increment is
\begin{equation}
\mathrm{d} W = -P_{\mathrm{mech}} \mathrm{d} V + P_{\mathrm{f}} \mathrm{d} V_{\mathrm{f}} \ ,
\end{equation}
So during some deformation that takes $P_{\mathrm{mech}}$ from
$P_{\mathrm{mech}}^{i}$ to $P_{\mathrm{mech}}^{f}$, and $P_{\mathrm{f}}$ from
$P_{\mathrm{f}}^{i}$ to $P_{\mathrm{f}}^{f}$, the total work is
\begin{equation}
\begin{aligned}
W & = -\int P_{\mathrm{mech}} \mathrm{d}V + \int P_{\mathrm{f}} \mathrm{d}V_{\mathrm{f}}\ ,\\
& = \frac{V}{K}\int_{P_{\mathrm{mech}}^{i}}^{P_{\mathrm{mech}}^{f}}P_{\mathrm{mech}} \mathrm{d}P_{\mathrm{mech}} - \frac{V{\alpha_{B}}}{K}\int_{P_{\mathrm{f}}^{i}}^{P_{\mathrm{f}}^{f}}P_{\mathrm{mech}} \mathrm{d}P_{\mathrm{f}} \\
& + V_{\mathrm{f}}A_{ii}\int_{P_{\mathrm{mech}}^{i}}^{P_{\mathrm{mech}}^{f}}P_{\mathrm{f}} \mathrm{d}P_{\mathrm{mech}} + V_{\mathrm{f}}A_{ii}B\int_{P_{\mathrm{f}}^{i}}^{P_{\mathrm{f}}^{f}} P_{\mathrm{f}} \mathrm{d}P_{\mathrm{f}} \ .
\end{aligned}
\end{equation}

Now consider two experiments:

- First take $P_{\mathrm{mech}}$ from $0$ to $P_{\mathrm{mech}}$ with
  $P_{\mathrm{f}}$ fixed at $0$.   Then,
  leaving $P_{\mathrm{mech}}$ fixed, take $P_{\mathrm{f}}$ from $0$ to
  $P_{\mathrm{f}}$.  The first takes work $VP_{\mathrm{mech}}^2/(2K)$,
  while the second takes work $-{\alpha_{B}} VP_{\mathrm{mech}} P_{\mathrm{f}}/K +
  V_{\mathrm{f}}A_{ii}B P_{\mathrm{f}}^{2}/2$.
- First take $P_{\mathrm{f}}$ from $0$ to $P_{\mathrm{f}}$ with
  $P_{\mathrm{mech}}$ fixed at $0$.   Then,
  leaving $P_{\mathrm{f}}$ fixed, take $P_{\mathrm{mech}}$ from $0$ to
  $P_{\mathrm{mech}}$.  The first takes work
  $V_{\mathrm{f}}A_{ii}B P_{\mathrm{f}}^{2}/2$, and the
  second takes work  $VP_{\mathrm{mech}}^{2}/(2K) +
  V_{\mathrm{f}}P_{\mathrm{mech}} A_{ii}P_{\mathrm{f}}$.

The two experiments must give the same work done (this is called the
Betti-Maxwell reciprocal theorem), which yields
\begin{equation}
\label{eq:tildek}
A_{ij} = \alpha_{B} C_{ijkl}\delta_{kl}/\phi \ .
\end{equation}
(This is Detournay and Cheng Eqn$~$(22).)

Now to identify $B$.  Consider a so-called *ideal porous material*, which is characterised by a
fully-connected pore space and a homogeneous and isotropic matrix material.  In this case, applying a
uniform porepressure, $P_{f}$, and an equal mechanical pressure, $P_{\mathrm{mech}}=P_{f}$, the solid
material will experience a uniform pressure throughout its skeleton.  This means it will deform
uniformly without any shape change, and
\begin{equation}
\frac{\Delta V_{\mathrm{f}}}{V_{\mathrm{f}}} = \frac{\Delta V}{V} \ .
\end{equation}
Substituting this equation, this specific pressure condition, and
[eq:tildek] into [eq:fund_volstrain] and [eq:deltavf], yields
\begin{equation}
B = 1 + \phi - \phi/{\alpha_{B}} \ .
\end{equation}
(This is Detournay and Cheng Eqns$~$(22), (24b), (37) and the definition of $\alpha_{B}$ in Table 1.)

Now that $A_{ij}$ and $B$ have been identified, they may be
substituted into [eq:deltavf].  Rearranging yields
\begin{equation}
\frac{\Delta V_{\mathrm{f}}}{V_{\mathrm{f}}} =
\frac{\alpha_{B}}{\phi}\delta_{ij}C_{ijkl} \left[ \Delta
\sigma_{kl}^{\mathrm{tot}} + \alpha_{B} \delta_{kl} \Delta P_{\mathrm{f}}
\right] +
\frac{\delta_{ij}\delta_{kl}C_{ijkl}}{\phi}(1-\alpha_{B})(\alpha_{B}-\phi)\Delta
P_{f}
\end{equation}
Using the expression for $\Delta V/V$ yields
\begin{equation}
\Delta V_{\mathrm{f}} = V\alpha_{B}\Delta\epsilon_{ii} +
V\delta_{ij}\delta_{kl}C_{ijkl}(1-\alpha_{B})(\alpha_{B}-\phi)\Delta P_{f}
\end{equation}
Now $\Delta\phi = V^{-1}\Delta V_{\mathrm{f}} -
V_{\mathrm{f}}V^{-2}\Delta V$, so using the definition of $K$
yields
\begin{equation}
\frac{\partial \phi}{\partial t} = (\alpha_{B} - \phi)\frac{\partial
  \epsilon_{ii}}{\partial t} + \frac{(1-\alpha_{B})(\alpha_{B} -
  \phi)}{K}\frac{\partial P_{\mathrm{f}}}{\partial t} \ ,
\end{equation}
as written in [eq:phi_dog].

## Evolution of porosity due to mineral precipitation

Consider now the case without any fluid porepressure or temperature, but with mineral precipitation.

The concentration of a mineral species, $C_{\mathrm{m}}$ is defined to be $V_{\mathrm{m}}/V$, where
the numerator is the volume of the mineral species and the denominator is the reference volume.
Suppose that $V = V_{\mathrm{m}} + V_{\mathrm{f}}$ and the definition of porosity is $\phi =
V_{\mathrm{f}}/V$.

Now consider volume changes via $\dot{V} = \dot{\epsilon}V$ (where $\epsilon$ is the volumetric
strain) and changes in the volume of mineral, $\dot{V}_{\mathrm{m}}$.  Under these changes, the
porosity evolves as
\begin{equation}
\dot{\phi} = \frac{\dot{V}_\mathrm{f}}{V} - \frac{V_\mathrm{f}\dot{V}}{V^{2}} = \frac{\dot{V}-\dot{V}_{\mathrm{m}}}{V} - \phi\dot{\epsilon}
\end{equation}
This yields, after a little algebra,
\begin{equation}
\dot{\phi} + \dot{C}_{\mathrm{m}} = \left(1 - (\phi + C_{\mathrm{m}}) \right) \dot{\epsilon} \ ,
\end{equation}
as written in [eq:phi_dog].

!bibtex bibliography
