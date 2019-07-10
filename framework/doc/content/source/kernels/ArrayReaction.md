# ArrayReaction

## Description

This array kernel implements the following piece of a weak form:
\begin{equation}
(\vec{u}^\ast, \mathbf{R} \vec{u}),
\end{equation}
where $\vec{u}^\ast$ is the test function, $\vec{u}$ is the finite element solution and $\mathbf{R}$ is the reaction coefficients.

Similarly as showed in [ArrayDiffusion.md], we can rearrange it into
\begin{equation}
(\vec{u}^\ast, \mathbf{R} \vec{u}) = \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} \vec{w}_p\vec{u}_i^\ast \underline{\mathbf{R}_{\text{qp}} \vec{u}_{\text{qp}} b_{i,\text{qp}}},
\end{equation}
where the underlined term is the vector provided by [ArrayReaction::computeQpResidual](ArrayReaction.C).
Detailed explanations on the notations can be found in [ArrayDiffusion.md].

In general, the reaction coefficient $\mathbf{D}$ is a square matrix with the size of the number of components.
When it is a diagonal matrix, it can be represented by a vector.
In such a case, the components are not coupled with this array reaction kernel.
If all elements of the diffusion coefficient vector are the same, we can use a scalar reaction coefficient.
Thus this kernel gives users an option to set the type of diffusion coefficient with a parameter named as *reaction_coefficient_type*.
Users can set it to *scalar*, *array* or *full* cooresponding to scalar, diagonal matrix and full matrix respectively.
Its default value is *array*.

The local Jacobian can be found in the following equation:
\begin{equation}
(\vec{u}^\ast, \mathbf{R} \vec{u}) = \sum_{e} \sum_{i=1}^{N_{\text{dof}}} \sum_{j=1}^{N_{\text{dof}}} \sum_{\text{qp}=1}^{N_{qp}} (|J|w)_{\text{qp}} \vec{w}_p\vec{u}_i^\ast \underline{\mathbf{R}_{\text{qp}} b_{j,\text{qp}} b_{i,\text{qp}}} \vec{u}_j.
\end{equation}
The underlined part is the local Jacobian evaluated by [ArrayReaction::computeQpJacobian](ArrayReaction.C) and [ArrayReaction::computeQpOffDiagJacobian](ArrayReaction.C).

!syntax parameters /Kernels/ArrayReaction

!syntax inputs /Kernels/ArrayReaction

!syntax children /Kernels/ArrayReaction
