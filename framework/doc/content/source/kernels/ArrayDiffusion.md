# ArrayDiffusion

## Description

This array kernel implements the following piece of a weak form:

\begin{equation}
(\nabla \vec{\psi}_i, \mathbf{D} \nabla \vec{u}_h),
\end{equation}
where $\vec{\psi}_i$ is the test function, $\vec{u}_h is the finite element solution and $\mathbf{D}$ is the diffusion coefficients.
$\vec{u}_h$ is an array variable whose size is equal to the number of components of the array variable.
The size of the vector test function $\vec{\psi}_i$ is the same as the size of $\vec{u}_h$.
$i$ is the index of of the test function.
When operating on a local element, the number of test functions that have the local support on the element is also called the number of local degrees of freedom.
It is noted that the test functions of all components are identical because all components of an array variable share the same finite element family and order.
In general, the diffusion coefficient $\mathbf{D}$ is a square matrix with the size of the number of components.
When it is a diagonal matrix, it can be represented by a vector.
In such a case, the components are not coupled with this array diffusion kernel.
If all elements of the diffusion coefficient vector are the same, we can use a scalar diffusion coefficient.
Inner product of two vectors $(\cdot, \cdot)$ here has element-wise inner products.

The Jacobian is
\begin{equation}
\int_e \mathbf{D}\nabla \phi_i\cdot\nabla \psi_j\,dx.
\end{equation}

!listing tests/kernels/array_kernels/array_diffusion_test.i block=Kernels

!syntax parameters /Kernels/ArrayDiffusion

!syntax inputs /Kernels/ArrayDiffusion

!syntax children /Kernels/ArrayDiffusion
