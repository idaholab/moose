# NestedKKSMultiACBulkC

!syntax description /Kernels/NestedKKSMultiACBulkC

Kim-Kim-Suzuki (KKS) nested solve kernel (1 of 3) for multiphase models. An Allen-Cahn kernel for the terms with a direct composition dependence. This kernel can be used for one or multiple species.

### Residual

$/eta_p$ is the nonlinear variable of this kernel. For a model with $N$ phases:

\begin{equation}
R=-\frac{\partial F_1}{\partial c_{i,1}}\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p}c_{i,j}
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta_p$. We need to apply the chain rule and will only keep terms
with the $\frac{\partial \eta_p}{\partial u_j}\frac{\partial}{\partial \eta_p} = \phi_j \frac{\partial}{\partial \eta_p}$ derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial\eta_p} (-\frac{\partial F_1}{\partial c_{i,1}}\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p}c_{i,j})    \\
&=&-\phi_j \left[\frac{\partial^2F_1}{\partial c_{i,1}^2}\frac{\partial c_{i,1}}{\partial\eta_p} \sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p}c_{i,j} + \frac{\partial F_1}{\partial c_{i,j}} \sum_{j=1}^N(\frac{\partial^2h(\eta_j)}{\partial\eta_p^2} + \frac{\partial h(\eta_j)}{\partial\eta_p}\frac{\partial c_{i,j}}{\partial\eta_p})  \right]
\end{aligned}
\end{equation}

#### Off-diagonal

Since the partial derivative of phase concentrations $c_{i,j}$ w.r.t global concentration $c_q$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv c_q$, the derivative w.r.t $c_q$ must be calculated separetely from any other variable dependence. We need to
apply the chain rule and will again only keep terms with the
$\frac{\partial c_q}{\partial u_j}\frac{\partial}{\partial c_q} = \phi_j \frac{\partial}{\partial c_q}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial c_q} (-\frac{\partial F_1}{\partial c_{i,1}}\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p}c_{i,j}) \\
&=& -\phi_j(\frac{\partial^2 F_1}{\partial c_{i,1}^2}\frac{\partial c_{i,j}}{\partial c_q} \sum_{j=1}^N\frac{h(\eta_j)}{\partial\eta_p}c_{i,j} + \frac{\partial F_1}{\partial c_{i,1}} \sum_{j=1}^N\frac{\partial h_j}{\partial\eta_p}\frac{\partial c_{i,j}}{\partial c_q})
\end{aligned}
\end{equation}

If $F_a$ contains any other *explicit* variables, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial T} (-\frac{\partial F_1}{\partial c_{i,1}}\sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p}c_{i,j}) \\
&=& -\phi_j\frac{\partial^2 F_1}{\partial c_{i,1}\partial T} \sum_{j=1}^N\frac{\partial h(\eta_j)}{\partial\eta_p}c_{i,j}
\end{aligned}
\end{equation}

The off-diagonal Jacobian contributions are multiplied by the Allen-Cahn
mobility $L$ at each point for consistency with the other terms in the Allen-Cahn
equation.

!syntax parameters /Kernels/NestedKKSMultiACBulkC

!syntax inputs /Kernels/NestedKKSMultiACBulkC

!syntax children /Kernels/NestedKKSMultiACBulkC
