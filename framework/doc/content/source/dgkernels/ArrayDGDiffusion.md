# ArrayDGDiffusion

## Description

This array DG (discontinuous Galerkin) kernel implements the following piece of a weak form:

\begin{equation}
\left([\![ {\vec{u}^\ast} ]\!] , [\![ \vec{\kappa}\vec{u} ]\!] \right)_{\Gamma_\text{int}} +
\left([\![ \vec{u}^\ast ]\!], \{\!\!\{ \vec{D}\vec{\nabla}\vec{u}\cdot\vec{n} \}\!\!\}\right)_{\Gamma_\text{int}} +
 \epsilon\left( \{\!\!\{ \vec{D}\vec{\nabla}\vec{u}^\ast\cdot\vec{n} \}\!\!\}, [\![ \vec{u} ]\!]\right)_{\Gamma_\text{int}},
\end{equation}
where $\vec{u}^\ast$ is the test function, $\vec{u}$ is the finite element solution and $\vec{D}$ is the diffusion coefficients for all components of the array variable.
$[\![ \cdot ]\!]$ and $\{\!\!\{ \cdot \}\!\!\}$ are the jump and average of the enclosed quantity on the internal sides.

$\vec{n}(x)$ is a unit norm defined on internal sides denoted by $\Gamma_\text{int}$.
$\epsilon$ can be 1, -1, and 0, corresponding symmetric, asymetric and incomplete interior penalty methods respectively.
The penalty coefficients $\kappa$ are evaluated with the following formulation:
\begin{equation}
\vec{\kappa} = \sigma \{\!\!\{ p^2\frac{\vec{D}}{h_\bot} \}\!\!\}, \quad  x\in\Gamma_\text{int},
\end{equation}
where $p$ is the polynomial order of the shape functions; $h_\bot$ is the length of the element orthogonal to the side; $\sigma$ is an adjustable constant.

!syntax parameters /DGKernels/ArrayDGDiffusion

!syntax inputs /DGKernels/ArrayDGDiffusion

!syntax children /DGKernels/ArrayDGDiffusion
