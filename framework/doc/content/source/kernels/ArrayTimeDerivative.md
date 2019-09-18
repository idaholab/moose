# ArrayTimeDerivative

## Description

This array kernel implements the following piece of a weak form:
\begin{equation}
(\vec{u}^\ast, \mathbf{T} \dot{\vec{u}}),
\end{equation}
where $\vec{u}^\ast$ is the test function, $\dot{\vec{u}}$ is time derivative of the array of finite element solutions ($\dot{\vec{u}} = \left[\frac{\partial u_1}{\partial t},\frac{\partial u_2}{\partial t},...\right]^T$), and $\mathbf{T}$ is a matrix of the time derivative coefficients ($(\mathbf{T})_{n,m} = T_{n,m}$).

Similarly as showed in [ArrayDiffusion.md], we can rearrange it into
\begin{equation}
(\vec{u}^\ast, \mathbf{T} \dot{\vec{u}}) = \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} \vec{w}_p\vec{u}_i^\ast \underline{\mathbf{T}_{\text{qp}} \dot{\vec{u}}_{\text{qp}} b_{i,\text{qp}}},
\end{equation}
where the underlined term is the vector provided by [ArrayTimeDerivative::computeQpResidual](ArrayTimeDerivative.C).
Detailed explanations on the notations can be found in [ArrayDiffusion.md].

In general, the reaction coefficient $\mathbf{T}$ is a square matrix with the size of the number of components.
When it is a diagonal matrix, it can be represented by a vector.
In such a case, the components are not coupled with this array time derivative kernel.
If all elements of the time derivative coefficient vector are the same, we can use a scalar reaction coefficient.
Thus this kernel gives users an option to set the coefficient to a scalar, vector, or matrix material property, corresponding to scalar, diagonal matrix, and full matrix, respectively.

The local Jacobian can be found in the following equation:
\begin{equation}
J_{n,m,i,j} = \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{j=1}^{N_{\text{dof}}} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} \vec{w}_p u_{n,i}^\ast \underline{T_{n,m,\text{qp}} b_{j,\text{qp}} b_{i,\text{qp}} \frac{\partial \dot{u}_{m,j}}{\partial u_{m,j}}},
\end{equation}
where $n$ and $m$ are the component row and column, respectively. The underlined part is the local Jacobian evaluated by [ArrayTimeDerivative::computeQpJacobian](ArrayTimeDerivative.C) and [ArrayTimeDerivative::computeQpOffDiagJacobian](ArrayTimeDerivative.C).

## Example Input Syntax

!listing tests/kernels/array_kernels/array_diffusion_reaction_transient.i block=Kernels

!syntax parameters /Kernels/ArrayTimeDerivative

!syntax inputs /Kernels/ArrayTimeDerivative

!syntax children /Kernels/ArrayTimeDerivative
