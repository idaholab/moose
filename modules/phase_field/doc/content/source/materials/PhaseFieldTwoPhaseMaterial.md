# PhaseFieldTwoPhaseMaterial

!syntax description /Materials/PhaseFieldTwoPhaseMaterial

Phase field method is an indicator function that distinguish the phases in multiphase systems. The `PhaseFieldTwoPhaseMaterial` kernel can be used to assign physical properties (e.g, density, viscosity,etc) based on the phase field variable. For e.g., in a particular system where there are two phases such that

\begin{equation}
    \phi= \begin{cases}-1 & \text { Phase 1 } \\ 1 & \text { Phase 2 } \\ -1<\phi<1 & \text { Interface between the phases }\end{cases}
\end{equation}

Then, this kernel can assign properties(for e.g, density) according to the following formula:

\begin{equation}
    rho(\phi)=\frac{1-\phi}{2} \rho_{\mathrm{1}}+\frac{1+\phi}{2} \rho_{\mathrm{2}}
\end{equation}

!syntax parameters /Materials/PhaseFieldTwoPhaseMaterial

!syntax inputs /Materials/PhaseFieldTwoPhaseMaterial

!syntax children /Materials/PhaseFieldTwoPhaseMaterial
