# FVCoupledForce

## Description

`FVCoupledForce` implements a source term within the domain $\Omega$ proportional to a coupled
variable:
\begin{equation}
\underbrace{-\sigma v}_{\textrm{FVCoupledForce}} + \sum_{i=1}^n \beta_i = 0 \in \Omega,
\end{equation}
where $\sigma$ is a known scalar coefficient, $v$ is a coupled unknown value, and the second term on
the left hand side corresponds to the strong forms of other kernels. In a species transport context,
the value $\sigma$ can be regarded as a reaction rate coefficient.

## Example Syntax

!listing test/tests/fvkernels/fv_coupled_var/coupled.i block=FVKernels

!syntax parameters /FVKernels/FVCoupledForce

!syntax inputs /FVKernels/FVCoupledForce

!syntax children /FVKernels/FVCoupledForce
