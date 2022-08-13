# INSFVEnergyTimeDerivative

## Description

The `INSFVEnergyTimeDerivative` kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\rho c_p \frac{\partial T}{\partial t}}_{\textrm{INSFVEnergyTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where $\rho$ is the material density, $c_p$ is the specific heat, $T$ is the fluid temperature and the
second term on the left hand side corresponds to the strong forms of other kernels.

The Jacobian is computed with automatic differentiation.

## Implementation

The derivative is obtained from the definition of the fluid energy. The isobaric and isochoric heat
capacities being equal for incompressible fluids,

\begin{equation}
U(T(t), p(t)) = \rho(T(t), p(t)) \int_0^T(t) dT cp(T(t), p(t))
\end{equation}

we take the partial derivative with regards to time:

\begin{equation}
\frac{\partial U}{\partial t} = \frac{\partial \rho(T, p)}{\partial t} \int_0^T dT cp(T, p) + \rho(T, p) \frac{\partial T}{\partial t} cp(T, p)
\end{equation}

The variation of the kinetic energy is not considered in this kernel.

!alert note
The specific energy, $u = \int_0^T dT cp(T, p)$, is currently approximated as $c_p T$.

!syntax parameters /FVKernels/INSFVEnergyTimeDerivative

!syntax inputs /FVKernels/INSFVEnergyTimeDerivative

!syntax children /FVKernels/INSFVEnergyTimeDerivative
