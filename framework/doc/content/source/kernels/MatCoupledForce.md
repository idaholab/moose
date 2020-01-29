# MatCoupledForce

## Description

`MatCoupledForce` implements a right hand side forcing term of the form:

\begin{equation}
 \text{PDE} = \sum\limits_{j=1}^n c_j p_j(t, \vec{x}) v_j(t, \vec{x}),
\end{equation}
where $c_j$ are fixed coefficients, $p_j$ are material properties, and $v_j$ are coupled variables.

The weak form, in inner-product notation, is defined as

## Example Syntax

The kernel block below models the situation where variable $u$ is computed by: $u = \sum\limits_{j=1}^n c_j p_j(t, \vec{x}) v_j(t, \vec{x})$.


!listing test/tests/kernels/material_coupled_force/material_coupled_force.i
         block=Kernels

The answer is verified with the postprocessor:

!listing test/tests/kernels/material_coupled_force/material_coupled_force.i
         block=Postprocessors


!syntax parameters /Kernels/MatCoupledForce

!syntax inputs /Kernels/MatCoupledForce

!syntax children /Kernels/MatCoupledForce
