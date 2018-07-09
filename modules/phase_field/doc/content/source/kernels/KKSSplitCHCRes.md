# KKSSplitCHCRes

!syntax description /Kernels/KKSSplitCHCRes

[`KKSSplitCHCRes`](/KKSSplitCHCRes.md) is the split version. In this kernel, we calculate the chemical
potential $\mu$ from $\frac{\partial F}{\partial c}$. The non-linear variable for this Kernel
is the concentration $c$. To calculate $\frac{\partial c}{\partial t}$ and
$\nabla \cdot M(c) \nabla \mu$, we use the [`CoupledTimeDerivative`](/CoupledTimeDerivative.md) and
[`SplitCHWRes`](/SplitCHWRes.md) kernels, respectively, as described
[here](phase_field/Phase_Field_Equations.md).

## Residual

In the residual routine we need to calculate the term $R= \frac{\partial F}{\partial c} - \mu$.
We exploit the KKS identity $\frac{\partial F}{\partial c}=\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}$
and arbitrarily use the a-phase instead.

\begin{equation}
R = \frac{\partial F_a}{\partial c_a} - \mu
\end{equation}

### Jacobian

#### On-Diagonal

Since there is no explicit dependence on the non-linear variable $c$ in the residual
equation, the diagonal components are zero.

#### Off-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv c_a$. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial c_a}{\partial u_j}\frac{\partial}{\partial c_a}=\phi_j \frac{\partial}{\partial c_a}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \frac{\partial R}{\partial u_j} = \phi_j \frac{\partial}{\partial c_a} \left( \frac{\partial F}{\partial c_a} - \mu \right)\\
&=& \phi_j  \frac{\partial^2F}{\partial c_a^2} \\
\end{aligned}
\end{equation}

For $\mu$

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial \mu} \left( \frac{\partial F}{\partial c_a} - \mu \right)\\
&=& -\phi_j \\
\end{aligned}
\end{equation}

!syntax parameters /Kernels/KKSSplitCHCRes

!syntax inputs /Kernels/KKSSplitCHCRes

!syntax children /Kernels/KKSSplitCHCRes
