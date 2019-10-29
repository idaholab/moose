## Divergence Theorem

The divergence theorem states that the volume integral of the divergence of a
vector field over a volume $V$ bounded by a surface $S$ is equal to the surface
integral of the vector field projected on the outward facing normal of the
surface $S$.

\begin{equation}
\int_\Omega \nabla F dV = \int_{\partial\Omega} F\cdot\mathbf{n}dS
\end{equation}

## Product Rule

Product rule for the product of a scalar $a$ and a vector $\mathbf{b}$ is useful
to reduce the derivative order on an expression in conjunction with the
divergence theorem.

\begin{equation}
\nabla (a\mathbf{b}) = \nabla a \cdot \mathbf{b} + a \nabla\cdot\mathbf{b}
\end{equation}

Shuffle the terms (and note that this is valid for a vector $\mathbf{a}$ and a
scalar $b$ as well)

\begin{equation}
-\nabla a \cdot \mathbf{b}  =  a \nabla\cdot\mathbf{b} - \nabla (a\mathbf{b})
\end{equation}

\begin{equation}
-\nabla \cdot \mathbf{a} b  = \mathbf{a}\cdot \nabla b - \nabla (\mathbf{a}b)
\end{equation}

The right most term ($\nabla(\dots)$) can be transformed using the divergence
theorem. This can be used to effectively shift a derivative over to the test
function when building a residual.

## Fundamental Lemma of calculus of variations

For a functional

\begin{equation}
F[\rho] = \int f( \boldsymbol{r}, \rho(\boldsymbol{r}), \nabla\rho(\boldsymbol{r}) )\, d\boldsymbol{r}
\end{equation}

the functional derivative in the Cahn-Hilliard equation can be calculated using
the rule

\begin{equation}
\frac{\delta F}{\delta c} = \frac{\partial f}{\partial c} - \nabla \cdot \frac{\partial f}{\partial\nabla c}.
\end{equation}

Note that the above formula is only valid up to first order derivivatives (i.e.
$\nabla^1$). The general formula (required for some more advanced phase field
models) for a functional

\begin{equation}
F[\rho(\boldsymbol{r})] = \int f( \boldsymbol{r}, \rho(\boldsymbol{r}), \nabla\rho(\boldsymbol{r}), \nabla^{(2)}\rho(\boldsymbol{r}), \dots, \nabla^{(N)}\rho(\boldsymbol{r}))\, d\boldsymbol{r}
\end{equation}

with higher order derivatives is

\begin{equation}
\begin{aligned}
\frac{\delta F[\rho]}{\delta \rho} &{} = \frac{\partial f}{\partial\rho} - \nabla \cdot \frac{\partial f}{\partial(\nabla\rho)} + \nabla^{(2)} \cdot \frac{\partial f}{\partial\left(\nabla^{(2)}\rho\right)} + \dots + (-1)^N \nabla^{(N)} \cdot \frac{\partial f}{\partial\left(\nabla^{(N)}\rho\right)} \\
&{} =   \frac{\partial f}{\partial\rho} + \sum_{i=1}^N (-1)^{i}\nabla^{(i)} \cdot \frac{\partial f}{\partial\left(\nabla^{(i)}\rho\right)},
\end{aligned}
\end{equation}

where the vector $r \isin \mathcal{R}^n$, and $\nabla^{(i)}$ is a tensor whose
$n^i$ components are partial derivative operators of order $i$

\begin{equation}
\left [ \nabla^{(i)} \right ]_{\alpha_1 \alpha_2 \cdots \alpha_i} = \frac {\partial^{\, i}} {\partial r_{\alpha_1}  \partial r_{\alpha_2} \cdots \partial r_{\alpha_i} } \qquad \qquad \text{where} \quad  \alpha_1, \alpha_2, \cdots, \alpha_i = 1, 2, \cdots , n .
\end{equation}

## Weak form of the `ACInterface` Kernel

The term $L\nabla(\kappa\nabla\eta)$ is multiplied with the test function $\psi$
and integrated, yielding

\begin{equation}
\left(L\nabla(\kappa\nabla\eta),\psi\right) = \left(\nabla \cdot \underbrace{(\kappa\nabla\eta)}_{\equiv \mathbf{a}},\underbrace{L\psi}_{\equiv b}\right)
\end{equation}

we moved the $L$ over to the right and identify a vector term $\mathbf{a}$ and a
scalar term $b$. Then we use the third equality in the _Product rule_ section to
obtain

\begin{equation}
-\mathbf{a}\cdot \nabla b + \nabla (\mathbf{a}b).
\end{equation}

The last term is converted into a surface integral using the _Divergence
theorem_, tielding

\begin{equation}
- \left( \kappa\nabla\eta,\nabla(L\psi) \right)
+ \left< L\kappa \nabla\eta \cdot \vec n, \psi\right>
\end{equation}
