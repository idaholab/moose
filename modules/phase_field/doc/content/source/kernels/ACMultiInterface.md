# ACMultiInterface

!syntax description /Kernels/ACMultiInterface

Implements Allen-Cahn interface terms for a multiphase system. This includes
cross terms of the form

\begin{equation}
\sum_j\frac12\kappa_{ij}\left( \eta_i\nabla\eta_j - \eta_j\nabla\eta_i \right)^2
\end{equation}

where $\eta_i$ is the non-linear variable the kernel is acting on, $\eta_j$ (`etas`
are all non-conserved order parameters in the system, $\kappa_{ij}$ (`kappa_name`)
are the gradient energy coefficents, and $L_i$ (`mob_name`) is the scalar (isotropic)
mobility associated with the $\eta_i$ order parameter.

## Derivation

The interfacial free energy density $f_{int}$ is implemented following [!cite](Nest98) equations
 (7) and (8) (also see footnote 1)

\begin{equation}
f_{int} = \sum_{\substack{a,b \\ b\neq a}} \frac12 \kappa_{ab} \left| \eta_a\nabla\eta_b - \eta_b\nabla\eta_a\right|^2,
\end{equation}

Where the sum is taken over unique tuples _a,b_ (i.e. without the permutations _b,a_).
We take the functional derivative taken using the lemma

\begin{equation}
\frac{\delta f}{\delta\eta} = \frac{\partial f}{\partial\eta} - \nabla\frac{\partial f}{\partial\nabla\eta}.
\end{equation}

We obtain a one dimensional sum for each of the $\eta$-derivatives.

\begin{equation}
\begin{aligned}
\frac{\delta f_{int}}{\delta\eta_a} & = \sum_b \kappa_{ab} \left[ (\eta_a\nabla\eta_b - \eta_b\nabla\eta_a)\nabla\eta_b + \nabla\left((\eta_a\nabla\eta_b - \eta_b\nabla\eta_a)\eta_b\right) \right] \\
&= \sum_{\substack{b\\b\neq a}} \kappa_{ab} \left[ \underbrace{2(\eta_a\nabla\eta_b - \eta_b\nabla\eta_a)\nabla\eta_b}_{\text{order 1}} + \underbrace{\eta_b(\eta_a\nabla^2\eta_b - \eta_b\nabla^2\eta_a)}_{\text{order 2}} \right]
\end{aligned}
\end{equation}

We transform this expression into the weak form and see that the derivative order on the _order 2_ term has to be reduced by shifting a gradient onto the test function by applying the product rule

\begin{equation}
v \nabla\cdot\mathbf{w} = -\nabla v \cdot \mathbf{w}  + \nabla\cdot (v\mathbf{w}),
\end{equation}

after multiplying with the test function $\psi$ and integrating over the volume $\Omega$. We identify $v$ and $w$ as follows

\begin{equation}
\int_\Omega\psi\eta_b(\eta_a\nabla^2\eta_b - \eta_b\nabla^2\eta_a) =
\int_\Omega\underbrace{\psi\eta_a\eta_b}_{v_1}\nabla\cdot\underbrace{\nabla\eta_b}_{\mathbf{w}_1} -
\int_\Omega\underbrace{\psi\eta_b^2}_{v_2}\nabla\cdot\underbrace{\nabla\eta_a}_{\mathbf{w}_2}
\end{equation}

\begin{equation}
  \int_\Omega\left[ -\nabla(\psi\eta_a\eta_b)\cdot\nabla\eta_b\right]
- \int_\omega\left[-\nabla(\psi\eta_b^2)\cdot\nabla\eta_a\right]
+ \int_\Omega \nabla\cdot(\psi\eta_a\eta_b\nabla\eta_b)
- \int_\Omega \nabla\cdot(\psi\eta_b^2\nabla\eta_a).
\end{equation}

We get rid of the last two terms by applying the divergence theorem and obtain

\begin{equation}
\underbrace{
  \int_\Omega\left[ -\nabla(\psi\eta_a\eta_b)\cdot\nabla\eta_b\right]
- \int_\Omega\left[-\nabla(\psi\eta_b^2)\cdot\nabla\eta_a\right]
}_{\text{volume terms}}
+ \underbrace{
  \int_{\partial\Omega} \mathbf{n}\cdot(\psi\eta_a\eta_b\nabla\eta_b)
- \int_{\partial\Omega} \mathbf{n}\cdot(\psi\eta_b^2\nabla\eta_a).
}_{\text{boundary terms}}
\end{equation}

to convert them from volume to surface/boundary integrals. We again apply the product rule to expand the gradient of the product in the _volume terms_ and obtain

\begin{equation}
\int_\Omega\left[
 -\left(
\eta_a\eta_b\nabla\psi + \psi\eta_b\nabla\eta_a + \psi\eta_a\nabla\eta_b
\right) \cdot\nabla\eta_b\right]
- \int_\Omega\left[
-\left(
\eta_b^2\nabla\psi
+ 2\psi\eta_b\nabla\eta_b
\right)\cdot\nabla\eta_a\right].
\end{equation}

### Residual

The total residual $R_a$ is then

\begin{equation}
\begin{aligned}
R_a = L_a\sum_{\substack{b\\b\neq a}}\kappa_{ab}
\Bigg[ &\,&
\int_\Omega\left[
2\psi(\eta_a\nabla\eta_b - \eta_b\nabla\eta_a)\nabla\eta_b
\right]& \\
&+& \int_\Omega\left[
 -\left(
\eta_a\eta_b\nabla\psi + \psi\eta_b\nabla\eta_a + \psi\eta_a\nabla\eta_b
\right) \cdot\nabla\eta_b\right] & \\
&-& \int_\Omega\left[
-\left(
\eta_b^2\nabla\psi
+ 2\psi\eta_b\nabla\eta_b
\right)\cdot\nabla\eta_a\right]&
\Bigg].
\end{aligned}
\end{equation}

### On-diagonal Jacobian

The on-diagonal jacobian $J_a$ is obtained by taking the derivative with respect to $\eta_{aj}$, where $\frac{\partial \eta_a}{\partial \eta_{aj}} = \phi_j$ and $\frac{\partial \nabla\eta_{aj}}{\partial \eta_{aj}} = \nabla\phi_j$

\begin{equation}
\begin{aligned}
J_a = L_a\sum_{\substack{b\\b\neq a}}\kappa_{ab}
\Bigg[
&\,&\int_\Omega2\psi\left[
(\phi_j\nabla\eta_b - \eta_b\nabla\phi_j)\nabla\eta_b
\right]& \\
&+& \int_\Omega\left[
-\left(
\phi_j\eta_b\nabla\psi + \psi\eta_b\nabla\phi_j + \psi\phi_j\nabla\eta_b
\right)\cdot\nabla\eta_b
\right]& \\
&-& \int_\Omega\left[
-\left(
\eta_b^2\nabla\psi
+ 2\psi\eta_b\nabla\eta_b
\right)\cdot\nabla\phi_j
\right]& \Bigg].
\end{aligned}
\end{equation}

### Off-diagonal jacobian

For the off diagonal Jacobian entry $J_{ab}$ we take the derivative $\frac{\partial R_a}{\partial \eta_{bj}}$ and obtain

\begin{equation}
\begin{aligned}
J_{ab} =
&\,&
L_a\kappa_{ab}\int_\Omega2\psi\left[
(\eta_a\nabla\phi_j - \phi_j\nabla\eta_a)\nabla\eta_b
+ (\eta_a\nabla\eta_b - \eta_b\nabla\eta_a)\nabla\phi_j
\right] \\
&+& \int_\Omega\left[
-\left( \eta_a\phi_j\nabla\psi + \psi\phi_j\nabla\eta_a + \psi\eta_a\nabla\phi_j \right) \cdot\nabla\eta_b
-\left( \eta_a\eta_b\nabla\psi + \psi\eta_b\nabla\eta_a + \psi\eta_a\nabla\eta_b \right) \cdot\nabla\phi_j
\right] \\
&-& \int_\Omega\left[
%-\left( \eta_b^2\nabla\psi + 2\psi\eta_b\nabla\eta_b \right)\cdot\nabla\eta_a
-\left( 2\eta_b\phi_j\nabla\psi + 2\psi(\phi_j\nabla\eta_b + \eta_b\nabla\phi_j) \right)\cdot\nabla\eta_a
\right]
\end{aligned}\end{equation}

----

1) Note, that in the two-phase case with $\eta_b=1-\eta_a$ this reduces to

\begin{equation}
\begin{aligned}
f_{int}  & = \frac12 \kappa_{ab} \left| \eta_a\nabla(1-\eta_a) - (1-\eta_a)\nabla\eta_a\right|^2 \\
& = \frac12 \kappa_{ab} \left| -\eta_a\nabla\eta_a - \nabla\eta_a + \eta_a\nabla\eta_a\right|^2 \\
& = \frac12 \kappa_{ab} \left| \nabla\eta_a \right|^2,
\end{aligned}
\end{equation}

which is the familiar form implemented by [`ACInterface`](/ACInterface.md).

!syntax parameters /Kernels/ACMultiInterface

!syntax inputs /Kernels/ACMultiInterface

!syntax children /Kernels/ACMultiInterface
