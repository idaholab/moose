
# ADSwitchingFunctionMultiPhaseMaterial

!syntax description /Materials/ADSwitchingFunctionMultiPhaseMaterial


This material implements the switching function for a multi-phase,
multi-order parameter system as defined by [!cite](Moelans2011). For phase $\alpha$, the switching function is

\begin{equation}
 h_{\alpha} = \frac{\Sigma_i \eta_{{\alpha}_i}^2}{\Sigma_{\rho} \Sigma_i \eta_{{\rho}_i}^2}
\end{equation}

where $i$ indexes the grains of a phase and $\rho$ indexes the phases.

!syntax parameters /Materials/ADSwitchingFunctionMultiPhaseMaterial

!syntax inputs /Materials/ADSwitchingFunctionMultiPhaseMaterial

!syntax children /Materials/ADSwitchingFunctionMultiPhaseMaterial
