# ArrayBodyForce

## Description

`ArrayBodyForce` applies body forces specified with functions to an array variable:

\begin{equation}
(\vec{u}^\ast, -\vec{f}),
\end{equation}
where $\vec{u}^\ast$ is the test functions of all the components of the array variable $\vec{u}$, and $\vec{f}$ is the body force functions whose size must agree with the number of components of the array variable.
The Jacobian term for this kernel is zero since it is assumed that $\vec{f}$ **is not** a function of the unknown $\vec{u}$.

## Example Input Syntax

!listing tests/kernels/array_kernels/array_body_force.i block=Kernels

!syntax parameters /Kernels/ArrayBodyForce

!syntax inputs /Kernels/ArrayBodyForce

!syntax children /Kernels/ArrayBodyForce
