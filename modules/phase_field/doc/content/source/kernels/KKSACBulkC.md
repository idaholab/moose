# KKSACBulkC

!syntax description /Kernels/KKSACBulkC

KKS Allen-Cahn kernel for the terms with a direct composition dependence.
An instance of this kernel is needed for each solute species of the problem.

### Residual

\begin{equation}
\begin{aligned}
R&=&-\frac{dh}{d\eta}\left(-\frac{dF_a}{dc_a}(c_a-c_b)\right)\\
&=&\frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b)
\end{aligned}
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta$. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta} = \phi_j \frac{\partial}{\partial \eta}$
derivative.
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial \eta} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{d^2h}{d\eta^2}\phi_j\frac{dF_a}{dc_a}(c_a-c_b)\\
\end{aligned}
\end{equation}

#### Off-diagonal

Since $c_a$ and $c_b$ appear in the residual, their effect must be calculated
separately from any other variable dependence. For $c_a$, we are looking for the
$\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv c_a$. We need to
apply the chain rule and will again only keep terms with the
$\frac{\partial c_a}{\partial u_j}\frac{\partial}{\partial c_a} = \phi_j \frac{\partial}{\partial c_a}$
derivative.
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial c_a} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{dh}{d\eta} \phi_j \left( \frac{d^2 F_a}{dc_a^2}(c_a-c_b) + \frac{d F_a}{d c_a}\right)\\
\end{aligned}
\end{equation}

Similarly for $c_b$,
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial c_b} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& -\frac{dh}{d\eta} \phi_j  \frac{d F_a}{d c_a}\\
\end{aligned}
\end{equation}

For any variable other than $c_a$ or $c_b$, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial T} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{dh}{d\eta} \phi_j  \frac{\partial}{\partial T}\left(\frac{d F_a}{d c_a}\right) ( c_a - c_b)\\
\end{aligned}
\end{equation}

The off-diagonal Jacobian contributions are again multiplied by the Allen-Cahn
mobility $L$ at each point for consistency with the other terms in the Allen-Cahn
equation.

!syntax parameters /Kernels/KKSACBulkC

!syntax inputs /Kernels/KKSACBulkC

!syntax children /Kernels/KKSACBulkC
