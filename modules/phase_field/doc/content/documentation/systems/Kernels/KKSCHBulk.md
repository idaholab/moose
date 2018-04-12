# KKSCHBulk

!syntax description /Kernels/KKSCHBulk

Non-split KKS Cahn-Hilliard bulk kernel, which is **not fully implemented**.
The non-linear variable for this Kernel is the concentration $c$.

### Residual

In the residual routine we need to calculate the term $R=\nabla \frac{dF}{dc}$.
We exploit the KKS identity $\frac{dF}{dc}=\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}$
and arbitrarily use the a-phase instead.
The gradient can be calculated through the chain rule (note that $F_a(c_a, p_1,p_2,\dots,p_n)$
is potentially a function of many variables).

\begin{equation}
R = \nabla \frac{dF_a}{dc_a} = \frac{d^2F_a}{dc_a^2}\nabla c_a + \sum_i \frac{d^2F_a}{dc_adp_i}\nabla p_i
\end{equation}

With $a = \{c_a, p_1,p_2,\dots,p_n\}$ being the vector of all arguments to $F_a$ this simplifies to

\begin{equation}
R=\sum_i \frac{d^2F_a}{dc_ada_i}\nabla a_i  = \sum_i R_i \nabla a_i
\end{equation}

using $R_i$ as a shorthand for the term $\frac{d^2F_a}{dc_ada_i}$ (and represented
in the code as the array `_second_derivatives[i]`). We do have access to the
gradients of $a_i$ through MOOSE (stored in `_grad\_args[i]`).

### Jacobian

The calculation of the Jacobian involves the derivative of the Residual term $R$
w.r.t. the individual coefficients $u_j$ of all parameters of $F_a$. Here $u$ can
stand for any variable $a_i$.

\begin{equation}
\frac{dR}{du_j} = \sum_i \left[ \frac d{du_j} R_i\nabla a_i \right] = \sum_i \left[  R_i\frac{d\nabla a_i}{du_j} + \nabla a_i \sum_k \frac {dR_i}{da_k}\frac{da_k}{du_j} \right]
\end{equation}

In the code $u$ is given by `jvar` for the off diagonal case, and $c$
(not $c_a$ or $c_b$!) in the on diagonal case.

#### Off-diagonal

Let's focus on off diagonal first. Here $\frac{da_k}{du_j}$ is zero, if `jvar`
is not equal $k$. Allowing us to remove the sum over $k$ and replace it with the
single non-zero summand

\begin{equation}
\frac{dR}{du_j} = \sum_i \left[  R_i\frac{d\nabla a_i}{du_j} + \nabla a_i \frac {dR_i}{da_\text{jvar}}\frac{da_\text{jvar}}{du_j} \right]
\end{equation}

In the first term in the square brackets the derivative $\frac{d\nabla a_i}{du_j}$
is only non-zero if $i$ is `jvar`. We can therefore pull this term out of the
sum.

\begin{equation}
\frac{dR}{du_j} = R_\text{jvar}\frac{d\nabla a_\text{jvar}}{du_j} + \sum_i  \nabla a_i \frac {dR_i}{da_\text{jvar}}\frac{da_\text{jvar}}{du_j}
\end{equation}

With the rules for $\frac d{du_j}$ derivatives we get

\begin{equation}
R_\text{jvar} \nabla \phi_j + \sum_i \nabla a_i \frac {dR_i}{da_\text{jvar}} \phi_j
\end{equation}

where $j$ is `_j` in the code.

#### On-diagonal

For the on diagonal terms we look at the derivative w.r.t. the components of the
non-linear variable $c$ of this kernel. Note, that $F_a$ is only indirectly a
function of $c$. We assume the dependence is given through $c(c_a)$. The chain
rule will thus yield terms of this form

\begin{equation}
\frac{dc_a}{dc} = \frac{\frac{d^2F_b}{dc_b^2}}{[1-h(\eta)]\frac{d^2F_b}{dc_b^2}+h(\eta)\frac{d^2F_a}{dc_a^2}},
\end{equation}

which is given as equation (23) in KKS. Following the off-diagonal  derivation we get

\begin{equation}
\frac{d^2F_a}{dc_a^2}\frac{dc_a}{dc} \nabla \phi_j + \sum_i \nabla a_i \frac {dR_i}{dc_a} \frac{dc_a}{dc} \phi_j
\end{equation}

#### On-diagonal second approach

Let's get back to the original residual with $\frac{dF}{dc}$. Then

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{d}{dc} \nabla \frac{dF}{dc}\\
&=& \phi_j  \nabla \frac{d^2F}{dc^2} \quad,\quad \text{with (29) from KKS}\\
&=& \phi_j  \nabla \frac{\frac{d^2F_b}{dc_b^2}\frac{d^2F_a}{dc_a^2}}{  [1-h(\eta)]\frac{d^2F_b}{dc_b^2}+h(\eta)\frac{d^2F_a}{dc_a^2} }\\
\end{aligned}
\end{equation}

!syntax parameters /Kernels/KKSCHBulk

!syntax inputs /Kernels/KKSCHBulk

!syntax children /Kernels/KKSCHBulk
