# Reaction / ADReaction

## Description

`Reaction` (and its automatic differentiation version, `ADReaction`) implements
a simple first-order reaction term where the rate of reaction is directly proportional
to the governing variable $u$. Its weak form is given by
\begin{equation}
(\psi_i, \lambda u_h)
\end{equation}
where $\lambda$ is the rate coefficient.

`Reaction` can be used to help set-up variations of advection-diffusion-reaction
equations.

## Example Syntax

The syntax for `Reaction` is simple, only taking the `type` and `variable`
parameters. An example block is shown below for a diffusion-reaction equation:

!listing test/tests/fvkernels/fv_coupled_var/coupled.i block=Kernels

!alert note
There is no FV (finite volume) version of `Reaction`. If you wish to use FV, use
[/FVCoupledForce.md].

!syntax parameters /Kernels/Reaction

!syntax inputs /Kernels/Reaction

!syntax children /Kernels/Reaction
