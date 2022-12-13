# NestedKKSACBulkC

!syntax description /Kernels/NestedKKSACBulkC

Kim-Kim-Suzuki (KKS) nested solve kernel (1 of 3). An Allen-Cahn kernel for the terms with a direct composition dependence. This kernel can be used for one or multiple species.

### Residual

If a model has N species:

\begin{equation}
\begin{aligned}
R&=&-\frac{dh}{d\eta}\left[-\sum_{c=1}^N\frac{dF_a}{dc_a}(c_a-c_b)\right]\\
&=&\frac{dh}{d\eta}\left[\sum_{c=1}^N\frac{dF_a}{dc_a}(c_a-c_b)\right]
\end{aligned}
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta$. We need to apply the chain rule and will only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta} = \phi_j \frac{\partial}{\partial \eta}$ derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{\partial}{\partial\eta}\left[\frac{dh}{d\eta}\sum_{c=1}^N\left(\frac{\partial F_a}{\partial c_a}(c_a-c_b)\right)\right]    \\
&=&\phi_j\left[\frac{d^2h}{d\eta^2}\sum_{c=1}^N\left(\frac{\partial F_a}{\partial c_a}(c_a-c_b)\right) + \frac{dh}{d\eta}\frac{\partial}{\partial\eta}\left(\sum_{c=1}^N\frac{\partial F_a}{\partial c_a}(c_a-c_b)\right)  \right] \\
&=&\phi_j\left[\frac{dh}{d\eta^2}\sum_{c=1}^N\left(\frac{\partial F_a}{\partial c_a}(c_a-c_b)\right) + \frac{dh}{d\eta}\sum_{c=1}^N\left(\sum_{b=1}^N(\frac{\partial^2F_a}{\partial c_a \partial b_a}\frac{\partial b_a}{\partial\eta})(c_a-b_a) + \frac{\partial F_a}{\partial c_a}(\frac{\partial c_a}{\partial\eta} - \frac{\partial c_b}{\partial\eta})\right)\right]
\end{aligned}
\end{equation}

#### Off-diagonal

Since the partial derivative of phase concentrations $c_a$ w.r.t global concentration $c$ is hidden when computing the $\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv c$, the derivative w.r.t $c$ must be calculated separetely from any other variable dependence. We need to
apply the chain rule and will again only keep terms with the
$\frac{\partial c}{\partial u_j}\frac{\partial}{\partial c} = \phi_j \frac{\partial}{\partial c}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j\frac{dh}{d\eta}\frac{\partial}{\partial c}\left[\sum_{c=1}^N\left(\frac{\partial F_a}{\partial c_a}(c_a-c_b)\right)\right] \\
&=& \phi_j\frac{dh}{d\eta}\sum_{c=1}^N\left(\sum_{b=1}^N(\frac{\partial^2F_a}{\partial c_a\partial b_a}\frac{\partial b_a}{\partial c})(c_a-c_b) + \frac{\partial F_a}{\partial c_a}(\frac{\partial c_a}{\partial c} - \frac{\partial c_b}{\partial c})\right)
\end{aligned}
\end{equation}

If $F_a$ contains any other *explicit* variables, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial T} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{dh}{d\eta} \phi_j  \frac{\partial}{\partial T}\left(\frac{d F_a}{d c_a}\right) (c_a - c_b)\\
\end{aligned}
\end{equation}

The off-diagonal Jacobian contributions are multiplied by the Allen-Cahn
mobility $L$ at each point for consistency with the other terms in the Allen-Cahn
equation.

!syntax parameters /Kernels/NestedKKSACBulkC

!syntax inputs /Kernels/NestedKKSACBulkC

!syntax children /Kernels/NestedKKSACBulkC
