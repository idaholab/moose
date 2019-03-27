# PenaltyInclinedBC

!syntax description /BCs/PenaltyInclinedBC

## Description

`PenaltyInclinedBC` is a `IntegratedBC` used for enforcing inclined boundary conditions $\mathbf{u}\cdot \mathbf{normal} = 0$. With a penalty method, the residual is given as
\begin{equation}
\mathcal{R}_i = \alpha(\mathbf{u}\cdot \mathbf{normal})\mathbf{normal}(\text{component})\psi_i
\end{equation}
where $\alpha$ is the penalty number and `component` corresponds to the direction in which to apply the residual.

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/inclined_bc/inclined_bc_2d.i block=BCs/right_x

!syntax parameters /BCs/PenaltyInclinedBC

!syntax inputs /BCs/PenaltyInclinedBC

!syntax children /BCs/PenaltyInclinedBC
