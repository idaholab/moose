# PorousFlowCapillaryPressureVG

!syntax description /UserObjects/PorousFlowCapillaryPressureVG

van Genuchten's capillary-pressure relationship [!citep](vangenuchten1980)

\begin{equation}
S_{\mathrm{eff}} =
\begin{cases}
1 & \textrm{if } P \geq 0 \ , \\
(1 + (-\alpha P)^{1/(1-m)})^{-m} & \textrm{if } P < 0 \ .
\end{cases}
\end{equation}
or
\begin{equation}
P_{c} =
\begin{cases}
0 & \textrm{if } S_{\mathrm{eff}} >= 1.0 \ , \\
\frac{1}{\alpha} (S_{\mathrm{eff}}^{-1/m} - 1)^{1 - m} & \textrm{if } S_{\mathrm{eff}} < 1
\end{cases}
\end{equation}

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


!bibtex bibliography
