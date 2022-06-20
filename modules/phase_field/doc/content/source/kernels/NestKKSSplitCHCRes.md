# NestKKSSplitCHCRes

!syntax description /Kernels/NestKKSSplitCHCRes

Kim-Kim-Suzuki (KKS) nested solve kernel (3 of 3). An kernel for the split Cahn-Hilliard term. This kernel operates on the global concentration $c$ as the non-linear variable.

## Residual

\begin{equation}
R = \frac{\partial F_a}{\partial c_a} - \mu
\end{equation}

### Jacobian

#### On-Diagonal

We need to apply the chain rule and will only keep terms
with the $\frac{\partial c}{\partial u_j}\frac{\partial}{\partial c}=\phi_j \frac{\partial}{\partial c}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial c}(\frac{\partial F_a}{\partial c_a} - \mu)   \\
&=& \phi_j\sum_{b=1}^N(\frac{\partial^2F_a}{\partial c_a\partial b_a}\frac{\partial b_a}{\partial c})
\end{aligned}
\end{equation}

#### Off-diagonal

Since the partial derivative of phase concentrations $c_a$ w.r.t phase parameter $eta$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv \eta$, the derivative w.r.t $\eta$ must be calculated separetely from any other variable dependence. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta}=\phi_j \frac{\partial}{\partial \eta}$ derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial\eta}(\frac{\partial F_a}{\partial c_a} - \mu)   \\
&=& \phi_j\sum_{b=1}^N(\frac{\partial^2F_a}{\partial c_a\partial b_a}\frac{\partial b_a}{\partial\eta})
\end{aligned}
\end{equation}


If $F_a$ contains any other *explicit* variables, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial T}(\frac{\partial F_a}{\partial c_a} - \mu)   \\
&=& \phi_j\frac{\partial}{\partial T}\frac{\partial F_a}{\partial c_a}
\end{aligned}
\end{equation}

!syntax parameters /Kernels/NestKKSSplitCHCRes

!syntax inputs /Kernels/NestKKSSplitCHCRes

!syntax children /Kernels/NestKKSSplitCHCRes
