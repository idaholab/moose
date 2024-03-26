# NestedKKSMultiSplitCHCRes

!syntax description /Kernels/NestedKKSMultiSplitCHCRes

Kim-Kim-Suzuki (KKS) nested solve kernel (3 of 3). A kernel for the split Cahn-Hilliard term. This kernel operates on the global concentration $c_i$ as the non-linear variable.

## Residual

\begin{equation}
R = \frac{\partial F_1}{\partial c_{i,1}} - \mu,
\end{equation}

where $c_{i,1}$ is the phase concentration of species $i$ in the first phase.

### Jacobian

#### On-Diagonal

We need to apply the chain rule and will only keep terms
with the $\frac{\partial c_i}{\partial u_j}\frac{\partial}{\partial c_i}=\phi_j \frac{\partial}{\partial c_i}$
derivative. If a system has C components:

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial c_i}(\frac{\partial F_1}{\partial c_{i,1}} - \mu)   \\
&=& \phi_j\sum_{j=1}^C(\frac{\partial^2F_1}{\partial c_{i,1}\partial c_{j,1}}\frac{\partial c_{j,1}}{\partial c_i})
\end{aligned}
\end{equation}

#### Off-diagonal

Since the partial derivative of phase concentrations $c_{i,1}$ w.r.t phase parameter $eta_p$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv \eta_p$, the derivative w.r.t $\eta_p$ must be calculated separately from any other variable dependence. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta_p}{\partial u_j}\frac{\partial}{\partial \eta_p}=\phi_j \frac{\partial}{\partial \eta_p}$ derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial\eta_p}(\frac{\partial F_1}{\partial c_{i,1}} - \mu)   \\
&=& \phi_j\sum_{j=1}^C(\frac{\partial^2 F_1}{\partial c_{i,1}\partial c{j,1}}\frac{\partial c_{j,1}}{\partial\eta_p})
\end{aligned}
\end{equation}


If $F_1$ contains any other *explicit* variables, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial T}(\frac{\partial F_1}{\partial c_{i,1}} - \mu)   \\
&=& \phi_j\frac{\partial^2 F_1}{\partial c_{i,1}\partial T }
\end{aligned}
\end{equation}

!syntax parameters /Kernels/NestedKKSMultiSplitCHCRes

!syntax inputs /Kernels/NestedKKSMultiSplitCHCRes

!syntax children /Kernels/NestedKKSMultiSplitCHCRes
