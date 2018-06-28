# Reaction

## Description

`Reaction` implements a simple first-order reaction term with unity rate
coefficient where the rate of reaction is directly proportional to the governing
variable $u$. Its weak form is given by
\begin{equation}
(\psi_i, u_h)
\end{equation}

`Reaction` can be used to help set-up variations of advection-diffusion-reaction
equations.

## Example Syntax

The syntax for `Reaction` is simple, only taking the `type` and `variable`
parameters. An example block is shown below for a diffusion-reaction equation:

!listing test/tests/dgkernels/2d_diffusion_dg/2d_diffusion_dg_test.i block=Kernels

!syntax parameters /Kernels/Reaction

!syntax inputs /Kernels/Reaction

!syntax children /Kernels/Reaction
