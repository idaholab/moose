# NestedKKSACBulkF

!syntax description /Kernels/NestedKKSACBulkF

Kim-Kim-Suzuki (KKS) nested solve kernel (2 of 3). An Allen-Cahn kernel for the terms without a direct composition dependence.

### Residual

\begin{equation}
R = -\frac{dh}{d\eta}(F_a-F_b) + w\frac{dg}{d\eta}
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta$. We need to apply the chain rule and will only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta}=\phi_j \frac{\partial}{\partial\eta}$
derivative.

\begin{equation}
\begin{aligned}
J &=& -\phi_j\frac{\partial}{\partial\eta}\left(\frac{dh}{d\eta}(F_a-F_b)\right) + w\phi_j\frac{\partial}{\partial\eta}\frac{dg}{dh}    \\
&=& -\phi_j\left(\frac{d^2h}{d\eta^2}(F_a-F_b) + \frac{dh}{d\eta}\sum_{c=1}^N(\frac{\partial F_a}{\partial c_a}\frac{\partial c_a}{\partial\eta} - \frac{\partial F_b}{\partial c_b}\frac{\partial c_b}{\partial\eta})\right) + w\phi_j\frac{d^2g}{d\eta^2}
\end{aligned}
\end{equation}

#### Off-diagonal

Since the partial derivative of phase concentrations $c_a$ w.r.t global concentration $c$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv c$, the derivative w.r.t $c$ must be calculated separetely from any other variable dependence. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial c}{\partial u_j}\frac{\partial}{\partial c}=\phi_j \frac{\partial}{\partial c}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial c}\left(-\frac{dh}{d\eta}(F_a-F_b) + w\frac{dg}{d\eta}\right)   \\
&=& -\phi_j\frac{dh}{d\eta}\sum_{c=1}^N(\frac{\partial F_a}{\partial c_a}\frac{\partial c_a}{\partial c} - \frac{\partial F_b}{\partial c_b}\frac{\partial c_b}{\partial c})
\end{aligned}
\end{equation}

If $F_a$ and $F_b$ contain any other *explicit* variables, for example temperature $T$:

\begin{equation}
J = -\phi_j\frac{dh}{d\eta}\left( \frac{\partial F_a}{\partial T} - \frac{\partial F_b}{\partial T}\right)
\end{equation}

The off-diagonal Jacobian contribution is multiplied by the Allen-Cahn mobility $L$ at each point for consistency with the other terms in the Allen-Cahn equation.

!syntax parameters /Kernels/NestedKKSACBulkF

!syntax inputs /Kernels/NestedKKSACBulkF

!syntax children /Kernels/NestedKKSACBulkF
