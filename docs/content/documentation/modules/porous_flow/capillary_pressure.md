#Capillary pressure

A number of capillary pressure curves are available in Porous Flow.

## Constant
[`PorousFlowCapillaryPressureConst`](/porous_flow/PorousFlowCapillaryPressureConst.md)

In this simple model, capillary pressure is constant
\begin{equation}
  P_c = c.
\end{equation}
This formulation is useful for testing purposes.

##van Genuchten
[`PorousFlowCapillaryPressureVG`](/porous_flow/PorousFlowCapillaryPressureVG.md)

van Genuchten's capillary-pressure relationship is \citep{vangenuchten1980}

\begin{eqnarray}
S_{\mathrm{eff}} & = & \left\{
\begin{array}{ll}
1 & \mbox{if } P \geq 0 \ , \\
(1 + (-\alpha P)^{1/(1-m)})^{-m} & \mbox{if } P < 0\ .
\label{eq:vg_cap}
\end{array}
\right.
\end{eqnarray}
or
\begin{eqnarray}
P_{c} & = & \left\{
\begin{array}{ll}
0 & \mbox{if } S_{\mathrm{eff}} >= 1.0 \ , \\
\frac{1}{\alpha} (S_{\mathrm{eff}}^{-1/m} - 1)^{1 - m} & \mbox{if }
S_{\mathrm{eff}} < 1
\end{array}
\right.
\end{eqnarray}

The effective saturation has been denoted by $S_{\mathrm{eff}}$ and
$P$ is the porepressure, which is the  *negative* of the capillary
pressure: $P = -P_{c}$.  Here $\alpha$ and $m$ are user-defined parameters.  The
parameter $m$ must satisfy
\begin{equation}
0 < m < 1 \ .
\end{equation}

Sometimes the van Genuchten function is defined in terms of the parameter
\begin{equation}
n = \frac{1}{1 - m} > 1 \ .
\end{equation}

In van Genuchten's paper, he finds good fits with experimental data
for various soils and rock when the parameter $m$ ranges between about
0.5 and 0.9 (meaning $2<n<10$, roughly), and $\alpha$ is between
$4\times 10^{-5}$ Pa$^{-1}$ and $2\times 10^{-4}$ Pa$^{-1}$.
\ref{van_genuchten_pc} shows the shape of the van Genuchten suction, $P_{c}$, as a function
of $S_{\mathrm{eff}}$.

Numerically there are three important features of
Eq. \eqref{eq:vg_cap}:

- $S_{\mathrm{eff}}$ is a monotonically decreasing function of
  $P_{c}$, which is necessary for a unique solution.
- $P_{c}\rightarrow \infty$ as $S_{\mathrm{eff}}\rightarrow 0$.  As mentioned
above, this is not justifiable physically, but numerically it is
extremely advantageous over $P_{c}\rightarrow P_{c}^{0}<\infty$, as
this latter version often causes algorithms to *get stuck* around
$S_{\mathrm{eff}} = 0$.  As also mentioned above, because of the low
relative permeability around $S_{\mathrm{eff}}$, physically realistic
problems rarely explore the $S_{\mathrm{eff}}\sim 0$ region.
- $S_{\mathrm{eff}}\rightarrow 1$ and
  $\mathrm{d}S_{\mathrm{eff}}/\mathrm{d}P_{c} \rightarrow 0^{+}$ as
  $P_{c}\rightarrow 0$, for all $m$.  This ensures that there is
  continuity in the porepressure, $P$, and the derivative
  $\mathrm{d}S/\mathrm{d}P$ around full saturation (remember that by definition
  $S_{\mathrm{eff}}=1$ for $P_{c}<0$).  Also
  $\mathrm{d}^{2}S_{\mathrm{eff}}/\mathrm{d}P_{c}^{2} \rightarrow 0$
  as $P_{c}\rightarrow 0^{+}$ if $m>0.5$.

!!! note:
    Users are encouraged to set *m* > 0.5

!media media/porous_flow/van_genuchten_pc.png width=80% margin-left=10px caption=Three values of $m$ are shown: 0.5, 0.7 and 0.9 id=van_genuchten_pc

##Broadbridge-White

The Broadbridge-White capillarity relationship valid for small $K_{n}$ is \citep{broadbridge1988}
\begin{equation}
S_{\mathrm{eff}} = S_{n} + (S_{s} - S_{n}) \frac{c}{1 + L(x)} \ .
\end{equation}
where
\begin{equation}
x = (c - 1) e^{c - 1 - c P/\lambda} \ ,
\end{equation}
and $L(x)$ is the Lambert W-function that satisfies $L(z)e^{L(z)}=z$.
This is of limited use in real simulations, and is only used in the Porous
Flow module for comparison with the analytical solutions of \citet{broadbridge1988} and
\citet{warrick1990} for multi-phase infiltration and drainage problems.

## Rogers-Stallybrass-Clements

The Rogers-Stallybrass-Clements capillary relationship is \citep{rsc1983}
\begin{equation}
S_{\mathrm{eff}} = \frac{1}{\sqrt{1 + \exp((P_{c} - A)/B)}} \ ,
\label{eqn.rsc.seff}
\end{equation}
when the oil viscosity is exactly twice the water viscosity.  This is
of limited use in real simulations, and is only used in the Porous
Flow module for comparison with the analytical solutions offered by
the authors for multi-phase infiltration and drainage problems.

##References
\bibliographystyle{unsrt}
\bibliography{docs/bib/porous_flow.bib}
