# KKSACBulkF

!syntax description /Kernels/KKSACBulkF

KKS Allen-Cahn kernel for the terms without a direct composition dependence.

### Residual

\begin{equation}
R=-\frac{dh}{d\eta}(F_a-F_b)+w\frac{dg}{d\eta}.
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta$. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta}=\phi_j \frac{\partial}{\partial\eta}$
derivative.

\begin{equation}
\begin{aligned}
J &=& -\phi_j \frac{\partial}{\partial \eta}\left( \frac{dh}{d\eta}(F_a-F_b) \right) + w \phi_j \frac{\partial}{\partial \eta}\frac{dg}{d\eta} \\
&=&-\frac{d^2h}{d\eta^2}\phi_j(F_a-F_b) + w\frac{d^2g}{d\eta^2}\phi_j \\
\end{aligned}
\end{equation}

(The implicit dependence of $F_a(c_a)$ and $F_b(c_a)$ on $\eta$ through $c_a(c,\eta)$
and $c_b(c,\eta)$ does not contribute to the Jacobian, so
$\frac{\partial F_a}{\partial \eta} = \frac{\partial F_a}{\partial \eta} = 0)$.

#### Off-Diagonal

The off-diagonal components are calculated for any other variables that $F_a$
and $F_b$ depend on. For example, for $c_a$:

\begin{equation}
J = \frac{dh}{d\eta}\left( \frac{\partial F_a}{\partial c_a} - \frac{\partial F_b}{\partial c_a}\right)\phi_j
\end{equation}

$\frac{\partial F_b}{\partial c_a} = 0$ in the KKS formulation, so this term
would not need to be included if $c_a$ was the only variable $F_a$ depended on.
However, the code calculates derivatives with respect to all variables that
$F_a$ and $F_b$ depend on in a general way so that the Jacobian entries for other
dependencies are correctly computed using the same piece of code. For example,
both $F_a$ and $F_b$ could depend on temperature $T$, in which case

\begin{equation}
J = \frac{dh}{d\eta}\left( \frac{\partial F_a}{\partial T} - \frac{\partial F_b}{\partial T}\right)\phi_j
\end{equation}

which is computed using the same code. The off-diagonal Jacobian contribution is
also multiplied by the Allen-Cahn mobility $L$ at each point for consistency with
the other terms in the Allen-Cahn equation.

!syntax parameters /Kernels/KKSACBulkF

!syntax inputs /Kernels/KKSACBulkF

!syntax children /Kernels/KKSACBulkF
