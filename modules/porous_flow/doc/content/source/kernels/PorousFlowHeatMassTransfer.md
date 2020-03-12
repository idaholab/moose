# PorousFlowHeatMassTransfer

!syntax description /Kernels/PorousFlowHeatMassTransfer

This `Kernel` implements the weak form of
\begin{equation*}
  C(u-v)
\end{equation*}
where $u$ and $v$ are `Variables` such as pressure, temperature or mass fraction. The kernel's `Variable` is $u$, while $v$ is the coupled `Variable`. $C$ is the transfer coefficient which defines the transfer from $v$ to $u$. Note, this `Kernel` does no mass lumping, which might effect the numerical stabilization.

!syntax parameters /Kernels/PorousFlowHeatMassTransfer

!syntax inputs /Kernels/PorousFlowHeatMassTransfer

!syntax children /Kernels/PorousFlowHeatMassTransfer
