#Porosity

Porosity may be fixed at a constant value, or it may be a function of the
effective porepressure, the volumetric strain and/or the temperature.

Available porosity formulations include:
## Constant
[`PorousFlowPorosityConst`](/porous_flow/PorousFlowPorosityConst.md)

The simplest case where porosity does not change during the simulation. A single
value of porosity can be used, or a spatially varying porosity `AuxVariable` can
be used to define a heterogeneous porosity distribution.

## Strain-dependent porosity
The porosity can also depend on the mechanical response of the porous medium using
one of the following materials (depending on the coupled physics)

- Hydro-Mechanical coupling: [`PorousFlowPorosityHM`](/porous_flow/PorousFlowPorosityHM.md)
- Thermo-Mechanical coupling: [`PorousFlowPorosityTM`](/porous_flow/PorousFlowPorosityTM.md)
- Hydro-Thermo-Mechanical coupling: [`PorousFlowPorosityTHM`](/porous_flow/PorousFlowPorosityTHM.md)

##Evolution of porosity
The evolution of the porosity is governed by \citep{chen2009}
\begin{equation}
\frac{\partial \phi}{\partial t} = (\alpha_{B} -
\phi)\frac{\partial}{\partial t}
\left(\epsilon^{\mathrm{total}}_{ii} - \alpha_{T} T\right) +
\frac{(1-\alpha_{B})(\alpha_{B}-\phi)}{K}\frac{\partial
  P_{f}}{\partial t} \ .
\label{eq:phi_dog}
\end{equation}
Here $K$ is the bulk modulus of the drained porous skeleton: $1/K
= \delta_{ij}\delta_{kl}C_{ijkl}$.  Also $\alpha_{T}$ is the volumetric coefficient of thermal expansion.  This has solution
\begin{equation}
\phi = \alpha_{B} + (\phi_{0} - \alpha_{B})\times \exp \left( \frac{\alpha_{B}
  - 1}{K}(P_{f} - P_{f}^{\mathrm{ref}}) - \epsilon^{\mathrm{total}}_{ii} + \alpha_{T}(T - T^{\mathrm{ref}}) \right) \ ,
\label{eq:poro_evolve}
\end{equation}
where $\phi_{0}$ is the porosity at zero porepressure, zero elastic
strain and zero temperature. Note this porosity can become negative,
and an option for ensuring positivity is detailed below.

Without porepressure effects, the correct expression for porosity as a
function of volumetric strain and temperature is
\begin{equation}
\phi = 1 + (\phi_{0} - 1)\times \exp \left(-
\epsilon^{\mathrm{total}}_{ii} + \alpha_{T}(T - T^{\mathrm{ref}}) \right) \ .
\label{eq:poroTM_evolve}
\end{equation}

These expressions may be modified to include theeffects of plasticity.

The exponential expressions Eq. \eqref{eq:poro_evolve} and Eq. \eqref{eq:poroTM_evolve} can yield
negative porosity values, which are unphysical.  To ensure positivity
of $\phi$, PorousFlow offers the following option.  First write both
equations in the form
\begin{equation}
\phi = A + (B - A) \exp(x) \ .
\end{equation}
In deriving specific forms for $A$, $B$ and $x$, above, it has been
assumed that $x$ is small.  Since physically $1\geq A > B > 0$, for
$x<0$ the porosity will be physically meaningful, but for $x\gg 0$,
$\phi$ can become negative.  For example, for positive volumetric
strain ($x<0$) the porosity is always physically meaningful.  However,
when the porous material is squashed with negative and large
volumetric strain ($x\gg 0$) the porosity can become negative.

Define
\begin{equation}
C = \log(A/(A-B)) \ .
\end{equation}
Then, for $x>0$, the above expression for $\phi$ is replaced by
\begin{equation}
\phi = A + (B - A) \exp\left(C (1 - \exp(-x/C)) \right)  \geq 0 \ .
\end{equation}
At first glance this expression may appear rather obscure.  It has
been constructed to satisfy the following requirements:

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

##Evolution of porosity for an isothermal situation

The evolution of porosity is fundamental to the coupling between flow
and solid mechanics.  Consider the isothermal situation with no
plasticity.

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
\frac{\Delta V}{V} = -\frac{1}{K}(P_{\mathrm{mech}} - \alpha_{B} \delta_{ij}\Delta P_{\mathrm{f}})
\ .
\label{eq:fund_volstrain}
\end{equation}
where the total mechanical pressure is
\begin{equation}
P_{\mathrm{mech}} = - \textrm{Tr}\sigma/3 \ .
\end{equation}
and $K$ is the so-called *drained* bulk modulus $K = \delta_{ij}C_{ijkl}\delta_{kl}$.
To find the evolution equation for porosity, a similar equation for
$\Delta V_{\mathrm{f}}/V_{\mathrm{f}}$ must be derived.

Assuming linearity
\begin{equation}
\frac{\Delta V_{\mathrm{f}}}{V_{\mathrm{f}}} = A_{ij}
(\Delta\sigma_{ij}^{\mathrm{tot}} + B \delta_{ij}\Delta
P_{\mathrm{f}}) \ .
\label{eq:deltavf}
\end{equation}
The Betti-Maxwell reciprocal theorem yields $A_{ij}$ and $B$, as is
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
A_{ij} = \alpha_{B} C_{ijkl}\delta_{kl}/\phi \ .
\label{eq:tildek}
\end{equation}

Now to identify $B$.  Consider a so-called
*ideal porous material*, which is characterised by a fully-connected
pore space and a homogeneous and isotropic matrix material.  In this
case, applying a uniform porepressure, $P_{f}$, and an equal
mechanical pressure, $P_{\mathrm{mech}}=P_{f}$, the solid material
will experience a uniform pressure throughout its skeleton.   This
means it will deform uniformly without any shape change, and
\begin{equation}
\frac{\Delta V_{\mathrm{f}}}{V_{\mathrm{f}}} = \frac{\Delta V}{V} \ .
\end{equation}
Substituting this equation, this specific pressure condition, and
Eq. \eqref{eq:tildek} into Eq's \eqref{eq:fund_volstrain}
and \eqref{eq:deltavf}, yields
\begin{equation}
B = 1 + \phi - \phi/{\alpha_{B}} \ .
\end{equation}

Now that $A_{ij}$ and $B$ have been identified, they may be
substituted into Eq. \eqref{eq:deltavf}.  Rearranging yields
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
as written in Eq. \eqref{eq:phi_dog}.

##References
\bibliographystyle{unsrt}
\bibliography{porous_flow.bib}
