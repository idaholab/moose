# NestedKKSMultiACBulkF

!syntax description /Kernels/NestedKKSMultiACBulkF

Kim-Kim-Suzuki (KKS) nested solve kernel (2 of 3) for multiphase models. An Allen-Cahn kernel for the terms without a direct composition dependence.

### Residual
$/eta_p$ is the nonlinear variable of this kernel. For a model with $N$ phases:

\begin{equation}
R = \sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial \eta_p}F_j + w_p\frac{dg_p}{d\eta_p}
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta_p$. We need to apply the chain rule and will only keep terms
with the $\frac{\partial \eta_p}{\partial u_j}\frac{\partial}{\partial \eta_p}=\phi_j \frac{\partial}{\partial\eta_p}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial\eta_p}(\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial \eta_p}F_j) + w_p\phi_j\frac{\partial}{\partial\eta_p}\frac{dg_p}{d\eta_p}    \\
&=& \phi_j\sum_{j=1}^N(\frac{\partial^2 h(\eta_j)}{\partial\eta_p^2} F_j + \frac{\partial h(\eta_j)}{\partial\eta_p} \frac{\partial F_j}{\partial c_{i,j}} \frac{\partial c_{i,j}}{\partial\eta_p}) + w_p\phi_j\frac{d^2g_p}{d\eta_p^2}
\end{aligned}
\end{equation}

#### Off-diagonal

Since the partial derivative of phase concentrations $c_{i,j}$ w.r.t global concentration $c_i$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv c_i$, the derivative w.r.t $c_i$ must be calculated separately from any other variable dependence. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial c_i}{\partial u_j}\frac{\partial}{\partial c_i}=\phi_j \frac{\partial}{\partial c_i}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial c_i}(\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial \eta_p}F_j + w_p\frac{dg_p}{d\eta_p})   \\
&=& \phi_j(\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p} \frac{\partial F_j}{\partial c_{i,j}} \frac{\partial c_{i,j}}{\partial c_i})
\end{aligned}
\end{equation}

Similarly, the partial derivative of phase concentrations $c_{i,j}$ w.r.t phase parameter $\eta_k$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv \eta_k$, the derivative w.r.t $\eta_k$ must be calculated separately from any other variable dependence. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta_k}{\partial u_j}\frac{\partial}{\partial \eta_k}=\phi_j \frac{\partial}{\partial \eta_k}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial \eta_k}(\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial \eta_p}F_j + w_p\frac{dg_p}{d\eta_p})   \\
&=& \phi_j\sum_{j=1}^N(\frac{\partial^2 h(\eta_j)}{\partial\eta_p \partial\eta_k}F_j + \frac{\partial h(\eta_j)}{\partial\eta_p} \frac{\partial F_j}{\partial c_{i,j}} \frac{\partial c_{i,j}}{\partial\eta_k})
\end{aligned}
\end{equation}

If $F_j$ contains any other *explicit* variables, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial T} \sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial \eta_p}F_j + w_p\frac{dg_p}{d\eta_p}  \\
&=& \phi_j\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p} \frac{\partial F_j}{\partial T}
\end{aligned}
\end{equation}

The off-diagonal Jacobian contribution is multiplied by the Allen-Cahn mobility $L$ at each point for consistency with the other terms in the Allen-Cahn equation.

!syntax parameters /Kernels/NestedKKSMultiACBulkF

!syntax inputs /Kernels/NestedKKSMultiACBulkF

!syntax children /Kernels/NestedKKSMultiACBulkF
