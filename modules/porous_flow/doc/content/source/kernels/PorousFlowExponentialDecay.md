# PorousFlowExponentialDecay

`!syntax description /Kernels/PorousFlowExponentialDecay`

## Description

The residual computed by `PorousFlowExponentialDecay` is
\begin{equation}
\mathrm{residual} = r(u - u_{\mathrm{ref}})
\end{equation}
where $u$ is the variable, and $r$ and $u_{\mathrm{ref}}$ are `AuxVariables` (or real values).  Combined with a `TimeDerivative` `Kernel`, this is useful for simulating exponential decay of $u$.  One example is simulating heat loss using the Lauwerier  model.

`!syntax parameters /Kernels/PorousFlowExponentialDecay`

`!syntax inputs /Kernels/PorousFlowExponentialDecay`

`!syntax children /Kernels/PorousFlowExponentialDecay`
