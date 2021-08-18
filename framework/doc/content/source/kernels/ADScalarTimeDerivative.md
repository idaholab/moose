# ADScalarTimeDerivative

This scalar kernel adds the time derivative of the paired scalar variable $u$ to
the nonlinear residual $R_u$:
\begin{equation}
  R_u = \frac{du}{dt} \,.
\end{equation}
The Jacobian of this residual contribution is computed using automatic differentiation.

!syntax parameters /ScalarKernels/ADScalarTimeDerivative

!syntax inputs /ScalarKernels/ADScalarTimeDerivative

!syntax children /ScalarKernels/ADScalarTimeDerivative
