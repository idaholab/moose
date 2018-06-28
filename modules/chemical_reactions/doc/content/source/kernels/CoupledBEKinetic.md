# CoupledBEKinetic

!syntax description /Kernels/CoupledBEKinetic

Derivative of concentration of $j^{\mathrm{th}}$ primary species present in the
mineral species wrt time. Implements the weak form of
\begin{equation}
\frac{\partial}{\partial t} \left(\phi \sum_m C_m \right),
\end{equation}
where $\phi$ is porosity and $C_m$ is the concentration of the $m^{\mathrm{th}}$
mineral species.

!syntax parameters /Kernels/CoupledBEKinetic

!syntax inputs /Kernels/CoupledBEKinetic

!syntax children /Kernels/CoupledBEKinetic
