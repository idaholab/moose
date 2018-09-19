<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# DiracKernels System

## Point Source

- Point sources (sometimes known as point loads) are typically modeled with Dirac distributions in finite element analysis.
- We will employ the following generalized form for the Dirac delta distribution acting at $x_0$:

\begin{equation*}
(\delta_{x_0} f, \psi) \equiv \int_{\Omega} \delta(x-x_0) f(x) \psi(x) \, \text{d}x = f(x_0) \psi(x_0)
\end{equation*}

where $f$ and $\psi$ are continuous functions.

- The Dirac delta distribution is thus an *integral operator* which "samples" the continuous functions $f$ and $\psi$ at the point $x_0$.
- The special case $\psi = 1$ is frequently used to induce the short-hand (abuse of) notation

\begin{equation*}
\delta_{x_0} f \equiv \int_{\Omega} \delta(x-x_0) f(x) \, \text{d}x = f(x_0)
\end{equation*}

- A diffusion equation with a point source/sink of strength $f$ (which can depend on $u$) located at $x_0$ can therefore be written as:

\begin{equation*}
-\nabla \cdot \nabla u - \delta_{x_0}f = 0
\end{equation*}

- Using th definiton above, the weak form for this equation (assuming Dirichlet BCs) is:

\begin{equation*}
(\nabla u, \nabla \psi_i) - (\delta_{x_0}f, \psi_i) = 0
\end{equation*}

- Assume the point $x_0$ falls in element $\Omega_e$. Then:

\begin{equation*}
(\delta_{x_0} f, \psi_i) = \int_{\Omega_e} \delta(x-x_0) f(x) \psi_i(x) \, \text{d}x = f(x_0) \psi_i(x_0)
\end{equation*}

- That is: we get a contribution for each DOF whose associated basis function $\psi_i$ is nonzero at $x_0$.
- The `DiracKernel` class computes these contributions given $x_0$ and $f$.

## Dirac Kernels

- A `DiracKernel` provides a residual (and optionally a Jacobian) at a set of points in the domain.
- The structure is very similar to kernels:

  - `computeQpResidual`/`computeQpJacobian()`
  - Parameters
  - Coupling
  - Material Properties
  - etc.

- The only difference is that `DiracKernel` *must* override the virtual `addPoints()` function to tell MOOSE the points at which the `DiracKernel` is active.
- Inside of `addPoints()` there are two different ways to tell MOOSE about the points:

  - `addPoint(Point p)` : Adds the physical point `p` that lies inside the domain of the problem.
  - `addPoint(Elem * e, Point p)` : Adds the physical point `p` that lies inside the element `e`.

- The second version is *much* more efficient if you know, a-priori, the element in which the point is located.

## (Some) Values Available to DiracKernels

- `_u, _grad_u` : Value and gradient of variable this DiracKernel is operating on.
- `_phi, _grad_phi` : Value ($\phi$) and gradient ($\nabla \phi$) of the trial functions at the current Dirac point.
- `_test, _grad_test` : Value ($\psi$) and gradient ($\nabla \psi$) of the test functions at the current Dirac point.
- `_q_point` : XYZ coordinates of the current Dirac Point.
- `_i, _j` : Current shape functions for test and trial functions respectively.
- `_current_elem` : A pointer to the current element that is being operated on.
- `_current_point` : The point where the DiracKernel is currently being asked to compute.

## Example 17

See [Example 17](ex17_dirac.md)

## Further DiracKernel Documentation

!syntax list /DiracKernels objects=True actions=False subsystems=False

!syntax list /DiracKernels objects=False actions=False subsystems=True

!syntax list /DiracKernels objects=False actions=True subsystems=False

