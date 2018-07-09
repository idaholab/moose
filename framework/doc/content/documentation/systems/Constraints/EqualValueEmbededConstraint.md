# EqualValueEmbededConstraint

This is a constraint acting upon overlapping portions of two blocks, a slave block and a master block. The constraint enforces the slave variable on the slave block and the master variable on the master block to have the same values. Mesh dimensions of two blocks do not have to match. Generally, the block with smaller mesh size should be the slave.

The constraint iterates through all the slave nodes on the slave block and finds associated master element. If a slave node has a master element, then contact force is computed according to user-specified contact model and formulation.

## Mathematical Introduction

The constraint utilizes contact models. Currently supported models and formulations are listed below.

## Model: Glued

For each pair of slave node and master element, the following constraint is enforced:
\begin{equation}
u_{slave} - u_{master} = 0
\end{equation}
where $u_{slave}$ is the value of the slave variable on the slave node, $u_{master}$ is a interpolation of the value of the master variable at the location of the slave node.

## Formulation: Kinematic

Contact force, slave residual and master residual are computed as:
\begin{equation}
\begin{aligned}
f_c &= -r_{s,copy}\\
r_s &= r_s + f_c + k_p(u_{slave} - u_{master})\\
r_m &= r_m - \phi_if_c
\end{aligned}
\end{equation}
where $r_{s,copy}$ is the ghosted residual and $k_p$ is the user-specified penalty parameter, it uses penalty parameter only to penalize error, converged solution has no error due to penalty compliance.

## Formulation: Penalty

Contact force, slave residual and master residual are computed as:
\begin{equation}
\begin{aligned}
f_c &= k_p(u_{slave} - u_{master})\\
r_s &= r_s + f_c\\
r_m &= r_m - \phi_if_c
\end{aligned}
\end{equation}
where $r_{s,copy}$ is the ghosted residual and $k_p$ is the user-specified penalty parameter. Small penalty may result in loose coupling between slave node and master element. Large penalty may lead to bad convergence.

!syntax description /Constraints/EqualValueEmbededConstraint

!syntax parameters /Constraints/EqualValueEmbededConstraint

!syntax inputs /Constraints/EqualValueEmbededConstraint

!syntax children /Constraints/EqualValueEmbededConstraint
