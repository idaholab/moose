# ArrayDiffusion

## Description

This array kernel implements the following piece of a weak form:

\begin{equation}
(\nabla \vec{u}^\ast, \mathbf{D} \nabla \vec{u}),
\end{equation}
which is expanded as
\begin{equation}
(\nabla \vec{u}^\ast, \mathbf{D} \nabla \vec{u}) = \sum_{p=1}^N w_p \sum_{q=1}^N (\nabla u_p^\ast, D_{p,q} \nabla u_q),
\end{equation}
where $\vec{u}^\ast$ is the test function, $\vec{u}$ is the finite element solution and $\mathbf{D}$ is the diffusion coefficients.
$\vec{u}$ is an array variable that has $N$ number of components.
$w_p, p=1,\cdots,N$ is the scalings of all components of the array variable.
The size of the vector test function $\vec{u}^\ast$ is the same as the size of $\vec{u}$.
The kernel can be furthur spelled out as
\begin{equation}
\sum_{p=1}^N w_p \sum_{q=1}^N (\nabla u_p^\ast, D_{p,q} \nabla u_q) = \sum_{p=1}^N w_p \sum_{q=1}^N \sum_{e} \int_e D_{p,q}(x) \nabla u_p^\ast (x) \cdot \nabla u_q(x)\,dx = \sum_{p=1}^N w_p \sum_{q=1}^N \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{j=1}^{N_{\text{dof}}} u_{p,i}^\ast u_{q,j} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} D_{p,q,\text{qp}} \nabla b_{i,\text{qp}} \cdot \nabla b_{j,\text{qp}}, \label{eq:weak-array-diffusion}
\end{equation}
where $e$ denotes elements of the mesh; $N_\text{dof}$ is the number of shape functions on the local element; $N_\text{qp}$ is the number of quadrature points for doing the spatial integration over an element. $u_{p,i}^\ast$ and $u_{p,i}^\ast$ are the expansion coefficients for the test function and the solution respectively, often referred as degrees of freedom. Subscript $\text{qp}$ indicates that the associated quantities are evaluated on a quadrature point. $(|J|w)_\text{qp}$ is the determinant of Jacobian, that transform a physical element to a reference element where the shape functions $b_i(\hat{x}),i=1,\cdots,N_\text{dof}$ are defined, times local quadrature weighting.
It is noted that the test functions of all components are identical because all components of an array variable share the same finite element family and order.

We can rearrange the fully expanded [eq:weak-array-diffusion] into
\begin{equation}
(\nabla \vec{u}^\ast, \mathbf{D} \nabla \vec{u}) = \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} \vec{w}_p\vec{u}_i^\ast \underline{\mathbf{D}_{\text{qp}} \nabla \vec{u}_{\text{qp}} \cdot \nabla b_{i,\text{qp}}},
\end{equation}
where the underlined term is the array provided by [ArrayDiffusion::computeQpResidual](ArrayDiffusion.C).
The element, shape function and quadrature point summations are taken care of by MOOSE.
It is noted that since test functions are arbitrary, $\vec{u}_i^\ast$ can be viewed as an index indicator with which the local residual goes into the global residual vector.
Note that $\vec{a}\vec{b}$ represents element-wise multiplication, i.e. $\vec{a}\vec{b}$ is equal to vector whose *i*th element is $a_i \times b_i$, where $a$ and $b$ are two generic vectors.

In general, the diffusion coefficient $\mathbf{D}$ is a square matrix with the size of the number of components.
When it is a diagonal matrix, it can be represented by an array.
In such a case, the components are not coupled with this array diffusion kernel.
If all elements of the diffusion coefficient vector are the same, we can use a scalar diffusion coefficient.
Thus this kernel gives users an option to set the type of diffusion coefficient with a parameter named as *diffusion_coefficient_type*.
Users can set it to *scalar*, *array* or *full* cooresponding to scalar, diagonal matrix and full matrix respectively.
Its default value is *array*.

With some further transformation, the kernel becomes
\begin{equation}
(\nabla \vec{u}^\ast, \mathbf{D} \nabla \vec{u}) = \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{j=1}^{N_{\text{dof}}} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} \vec{w}_p\vec{u}_i^\ast \underline{\mathbf{D}_{\text{qp}} \nabla b_{j,\text{qp}} \cdot \nabla b_{i,\text{qp}}} \vec{u}_j,
\end{equation}
where the underlined part is the local Jacobian evaluated by [ArrayDiffusion::computeQpJacobian](ArrayDiffusion.C) and [ArrayDiffusion::computeQpOffDiagJacobian](ArrayDiffusion.C).

## Example Input Syntax

!listing tests/kernels/array_kernels/array_diffusion_test.i block=Kernels

!syntax parameters /Kernels/ArrayDiffusion

!syntax inputs /Kernels/ArrayDiffusion

!syntax children /Kernels/ArrayDiffusion
