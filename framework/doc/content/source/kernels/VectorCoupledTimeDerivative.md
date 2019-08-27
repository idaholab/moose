# VectorCoupledTimeDerivative

!syntax description /Kernels/VectorCoupledTimeDerivative

## Overview

The `VectorCoupledTimeDerivative` kernel is very similar to the
[`VectorTimeDerivative`](/VectorTimeDerivative.md) kernel with the
exception that the time derivative operator is applied to a coupled variable $\vec{v}$ instead
of the variable $\vec{u}$ to whom's residual the `VectorCoupledTimeDerivative` kernel
contributes. Consequently, the strong form on the the domain $\Omega$ is

\begin{equation}
\underbrace{\frac{\partial \vec{v}}{\partial t}}_{\textrm{VectorCoupledTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega
\label{strong}
\end{equation}
where the second term on the left hand side corresponds to the
strong forms of other kernels. The `VectorCoupledTimeDerivative` weak form is then

\begin{equation}
R_i(\vec{u}_h) = \bigg(\vec{\psi}_i, \frac{\partial \vec{v}_h}{\partial t}\bigg) \quad \forall
\vec{\psi}_i,
\label{weak}
\end{equation}
where the $\vec{\psi}_i$ are test functions and $\vec{u}_h$ is the finite
element solution of the weak formulation.

The Jacobian contribution is equal to
\begin{equation}
\frac{\partial R_i(\vec{u}_h)}{\partial u_j} = (\vec{\psi}_i, a_v\vec{\phi}_j).
\end{equation}
where $a_v$ is a constant that depends on the time stepping scheme; $a_v$ is
denoted by `_dv_dot` in the `VectorCoupledTimeDerivative` class.

## Example Input File Syntax

!listing test/tests/kernels/coupled_time_derivative/vector_coupled_time_derivative_test.i block=Kernels

!syntax parameters /Kernels/VectorCoupledTimeDerivative

!syntax inputs /Kernels/VectorCoupledTimeDerivative

!syntax children /Kernels/VectorCoupledTimeDerivative
