# ADVectorTimeDerivative

## Description

The `ADVectorTimeDerivative` kernel implements a simple time derivative for the
domain $\Omega$ given by

\begin{equation}
\underbrace{\frac{\partial \vec u}{\partial t}}_{\textrm{ADVectorTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}

where the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `ADVectorTimeDerivative` weak form using
inner-product notation is

\begin{equation}
R_i(\vec u_h) = (\vec \psi_i, \frac{\partial \vec u_h}{\partial t}) \quad \forall \psi_i,
\end{equation}

where $\vec u_h$ is the approximate solution and $\vec \psi_i$ is a finite
element test function.

The Jacobian is computed through automatic differentiation. More information
about time kernels can be found on the Kernels description
[page](syntax/Kernels/index.md).

## Example Syntax

Time derivative terms are ubiquitous in any transient simulation. The kernel
block for a transient diffusion problem that demonstrates the
`ADVectorTimeDerivative` syntax is shown below:

!listing test/tests/kernels/ad_transient_diffusion/ad_transient_vector_diffusion.i block=Kernels

!syntax parameters /Kernels/ADVectorTimeDerivative

!syntax inputs /Kernels/ADVectorTimeDerivative

!syntax children /Kernels/ADVectorTimeDerivative
