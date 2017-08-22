# PorousFlowCapillaryPressureVG
!syntax description /UserObjects/PorousFlowCapillaryPressureVG

van Genuchten's capillary-pressure relationship \citep{vangenuchten1980}

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
$P$ is the porepressure, which is the *negative* of the capillary
pressure: $P = -P_{c}$.  Here $\alpha$ and $m$ are user-defined parameters.  The
parameter $m$ must satisfy
\begin{equation}
0 < m < 1 \ .
\end{equation}

By default, a logarithmic extension for low liquid phase saturations is implemented.
This can be disabled by setting `log_extension = false`.

!syntax parameters /UserObjects/PorousFlowCapillaryPressureVG

!syntax inputs /UserObjects/PorousFlowCapillaryPressureVG

!syntax children /UserObjects/PorousFlowCapillaryPressureVG

##References
\bibliographystyle{unsrt}
\bibliography{porous_flow.bib}
