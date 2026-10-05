# PenaltyInclinedNoDisplacementBC

!syntax description /BCs/PenaltyInclinedNoDisplacementBC

## Description

`PenaltyInclinedNoDisplacementBC` is a `IntegratedBC` used for enforcing inclined boundary conditions $\mathbf{u}\cdot \mathbf{normal} = 0$ for mechanics problems. With a penalty method, the residual is given as
\begin{equation}
\mathcal{R}_i = \alpha(\mathbf{u}\cdot \mathbf{normal})\mathbf{normal}(\text{component})\psi_i
\end{equation}
where $\alpha$ is the penalty parameter and `component` corresponds to the direction in which to apply the residual. The normal directly comes from the surface normal defined in a mesh.

[InclinedNoDisplacementConstraint.md] enforces the same condition exactly and without a penalty parameter, by adding one degree of freedom constraint row per node of the support instead of a residual, so it adds no stiffness and needs no $\alpha$ to be tuned against the scaling of the displacements. It builds the support normal once from the undisplaced mesh, which makes it the choice for a flat or locally flat support under small deformation. The penalty form here remains the right choice when the normal of the support turns as the body slides over it, which is the case of large sliding on a curved support.
