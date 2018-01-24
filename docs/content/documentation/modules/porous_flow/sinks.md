#Sources/sinks
A number of sources/sinks are available in Porous Flow, implemented as `DiracKernels`.

##Constant point source
[`PorousFlowSquarePulsePointSource`](/porous_flow/PorousFlowSquarePulsePointSource.md).

A constant mass point source that adds (removes) fluid at a constant mass flux
rate for times between the specified start and end times. If no start and end
times are specified, the source (sink) starts at the start of the simulation and
continues to act indefinitely.

##Line sinks

Polyline sinks and sources are modelled as sequences of discrete points:
\begin{equation}
\textrm{polyline}\sim \left\{x_{0},\ x_{1},\ x_{2},\ldots,x_{N}\right\} \ .
\end{equation}
The sink is
\begin{equation}
s = \sum_{i}f(P_{i})w_{i}\delta(x - x_{i}) \ .
\label{eq:line_sink}
\end{equation}

Here $s$ is a volume source, measured in kg.m$^{-3}$.s$^{-1}$ (or
J.m$^{-3}$.s$^{-1}$ for heat flow), which when integrated over the finite
element yields just the sink *strength*, $f$, which has units kg.s$^{-1}$ for
fluid flow, or J.s$^{-1}$ for heat flow.

The strength, $f$, is a function of porepressure or temperature, and may involve
other quantities, as enumerated below. The convention followed is:

- A sink has $s>0$.  This removes fluid or heat from the simulation domain;
- A source has $s<0$.  This adds fluid or heat to the simulation domain.

The input parameters for each PorousFlow line sink involve a plain text
file whose lines are space-separated quantities:
\begin{equation}
{\mathtt{w_{i}\ x_{i}\ y_{i}\ z_{i}}}
\label{eq:bh_plaintext_format}
\end{equation}

The weighting terms, $w_{i}$, are for user convenience, but for the Peaceman
borehole case they are the borehole radius at point $x_{i}$.

The basic sink may be multiplied by any or all of the following quantities

- Fluid relative permeability
- Fluid mobility ($k_{r} \rho / \nu$)
- Fluid mass fraction
- Fluid enthalpy
- Fluid internal energy

That is, $f$ in Eq. \eqref{eq:line_sink} may be replaced by $fk_{r}$,
$f\times\textrm{mobility}$, etc.  (The units of $fk_{r}$,
$f\times\textrm{mobility}$, etc, are kg.s$^{-1}$ for fluid flow, or J.s$^{-1}$ for
heat flow.)  All these additional multiplicative factors are evaluated at the
nodal positions, not at point $x_{i}$, to ensure superior numerical convergence
(see [upwinding](/porous_flow/upwinding.md)).

For instance, a fluid sink may extract fluid at a given rate, and therefore in a
simulation that includes, the same sink multiplied by fluid enthalpy should be
applied to the temperature variable.

###Poly-line sinks as functions of porepressure or temperature
[`PorousFlowPolyLineSink`](/porous_flow/PorousFlowPolyLineSink.md).

A poly-line sink is a special case of the general line sink.  The function, $f$
in Eq. \eqref{eq:line_sink} is assumed to be a piecewise linear function of
porepressure or temperature.  In addition, a multiplication by the line-length
associated to $x_{i}$ is also performed.  Specifically:

\begin{equation}
f(P_{i}) = \frac{1}{2}\left( |x_{i} - x_{i-1}| + |x_{i} - x_{i+1}|
\right) L(P_{i}) \ ,
\label{eq:pls}
\end{equation}
where $L$ is a piecewise-linear function, specified by the user in the MOOSE input file.

These types of sinks are useful in describing groundwater-surface water
interactions via streams and swamps.  Often a riverbed conductance, measured in
kg.Pa$^{-1}$.s$^{-1}$ is defined, which is

\begin{equation}
C = \frac{k_{zz}\rho}{L\mu}L_{\mathrm{seg}}W_{\mathrm{seg}} \ .
\end{equation}

Here $k_{zz}$ is the vertical component of the permeability tensor, $\rho$ is
the fluid density, $\mu$ is the fluid viscosity, and $L$ is a distance variable
related to the riverbed thickness.  The other parameters are $L_{\mathrm{seg}}$
and $W_{\mathrm{seg}}$, which are, respectively, the length and width of the
segment of river that the point $x_{i}$ is representing.  The multiplication by
$L_{\mathrm{seg}}$ is already handled by Eq. \eqref{eq:pls}, and the other terms
of $C$ will enter into the piecewise linear function, $L$.  Three standard types
of $L$ are used in groundwater models.

- A perennial stream, where fluid can seep from the porespace to the stream, and vice versa.  Then $L \propto (P - P_{\mathrm{atm}})$, where the proportionality constant involves the terms in $C$, and $P_{\mathrm{atm}}$ involves the river stage height;
- An ephemral stream, where fluid can only seep from the porespace to the stream, but not viceversa has $L\propto (P-P_{\mathrm{atm}})$ if $P>P_{\mathrm{atm}}$, and zero otherwise.  This is a pure sink since $s>0$ always;
- A rate-limited stream, where fluid can exchange between the groundwater and stream, but the rate is limited.  This can be modelled by using a piecewise linear $L$ that does not exceed given limits.

##Peaceman Boreholes
[`PorousFlowPeacemanBorehole`](/porous_flow/PorousFlowPeacemanBorehole.md)

Wellbores are implemented using the method first described by \citet{peaceman1983}.  Here $f$ is a
special function (measured in kg.s$^{-1}$ in standard units) defined in terms of the pressure at a
point at the wall of the wellbore (which is an input parameter)
\begin{equation}
P_{\mathrm{wellbore}}(x_{i}) = P_{\mathrm{bot}} + \gamma \cdot (x_{i} -
x_{i}^{\mathrm{bot}}) \ .
\end{equation}
The form of $f$ is
\begin{equation}
f(P_{i}, x_{i}) =
W \left|C(P_{i}-P_{\mathrm{wellbore}})\right|
\frac{k_{\mathrm{r}}\rho}{\mu}(P_{i} - P_{\mathrm{wellbore}})
\end{equation}

For boreholes that involve heat only, the $P$ in the above expressions should be
replaced by the temperature.

There are a number of parameters in these expressions:

- $P_{\mathrm{bot}}$ is an input parameter.  It is the pressure at
  the bottom of the wellbore;
- $x_{i}^{\mathrm{bot}}$ is the position of the bottom of the wellbore;
- $\gamma$ is a weight vector pointing downwards (product of fluid density and gravity).  This means that $P_{\mathrm{wellbore}}(x_{i})$ will be the pressure at point $x_{i}$ in the wellbore, due to gravitational head.  If these gravitational effects are undesirable, the user may simply specify $\gamma = (0,0,0)$;
- $k_{\mathrm{r}}$, $\rho$, and $\mu$ are the fluid relative permeability, density and viscosity.  Hence the term $k_{\mathrm{r}}\rho/\mu$ is the mobility, so users should choose to multiply by the mobility when using [`PorousFlowPeacemanBorehole`](/porous_flow/PorousFlowPeacemanBorehole.md).  Recall that all the multiplicative factors, including mobility, are evaluated at the nodal positions, not the position $x_{i}$.  This ensures superior numerical convergence;
- In the PorousFlow implementation $C$ is called the *character* of the wellbore.   There are two standard choices (note that $f$ depends only on the absolute value $|C|$):
    - $C=1$ for $P>P_{\mathrm{wellbore}}$, and zero otherwise.  In this case the wellbore is called a *production* wellbore, and it acts as a sink, removing fluid from the porespace.
    - $C=-1$ for $P<P_{\mathrm{wellbore}}$, and zero otherwise.  In this case the wellbore is called an *injection* wellbore, and it acts as a source, adding fluid to the porespace.
Generalising, $C$ may be chosen so that $|C|\neq 1$, which is useful for
time-varying borehole strengths.  The above two choices are
generalised to: if $C>0$ the wellbore is active only for
$P>P_{\mathrm{wellbore}}$ (and has zero strength otherwise); if $C<0$
the wellbore is active only for $P<P_{\mathrm{wellbore}}$.
- The function $W$ is called the well-constant of the wellbore, and is
measured in units of length$^{3}$.

Peaceman described the form of $W$ by performing 2D analytical and numerical
calculations to obtain the following formula

\begin{equation}
W = 2\pi \sqrt{\kappa_{xx}\kappa_{yy}}L_{z}/\ln(r_{e}/r_{\mathrm{bh}})
\ ,
\end{equation}

In this formula: the borehole is oriented along the $z$ direction; $\kappa_{xx}$
and $\kappa_{yy}$ are the diagonal components of the permeability tensor in the
$(x,y)$ plane; $L_{z}$ is the length of the borehole, $r_{\mathrm{bh}}$ is the
borehole radius (an input parameter, which is encoded in the plaintext file of
points: see Eq. \eqref{eq:bh_plaintext_format}); and, $r_{e}$ is the effective
borehole radius.  For a cell-centred finite-difference approach,
\citet{peaceman1983} found that

\begin{equation}
r_{e} = 0.28 \frac{\sqrt{\sqrt{\kappa_{xx}/\kappa_{yy}}L_{x}^{2} +
    \sqrt{\kappa_{yy}/\kappa_{xx}}L_{y}^{2}}}{(\kappa_{xx}/\kappa_{yy})^{1/4}
  + (\kappa_{yy}/\kappa_{xx})^{1/4}}
= 0.2 \frac{\sqrt{\frac{1}{2}\sqrt{\kappa_{xx}/\kappa_{yy}}L_{x}^{2} +
    \frac{1}{2}\sqrt{\kappa_{yy}/\kappa_{xx}}L_{y}^{2}}}{\frac{1}{2}(\kappa_{xx}/\kappa_{yy})^{1/4}
  + \frac{1}{2}(\kappa_{yy}/\kappa_{xx})^{1/4}} \ .
\end{equation}
Here $L_{x}$ and $L_{y}$ are the finite-difference spatial sizes.

Other authors have generalised Peaceman's approach to writing $W$ for different
geometrical situations.  Some of these are contained in \citet{chen2009}, where
they show that for a finite element situation with square elements of size $L$,
the borehole at a nodal position, and isotropic permeability

\begin{equation}
r_{e} =  0.113L \ .
\end{equation}

Note that the 0.113 is substantially different to Peaceman's 0.2, demonstrating
that this method of introducing wellbores dependent on the geometry.  The user
may specify this quantity via the `re_constant` input parameter (which defaults
to Peaceman's 0.28).

##References
\bibliographystyle{unsrt}
\bibliography{porous_flow.bib}
