# Numerical Integration

- The only remaining non-discretized parts of the weak form are the integrals.
- We split the domain integral into a sum of integrals over elements:
  $$$\int_{\Omega} f(\vec{x}) \;\text{d}\vec{x} = \sum_e \int_{\Omega_e} f(\vec{x}) \;\text{d}\vec{x}$$$
- Through a change of variables, the element integrals are mapped to integrals over the "reference" elements $$$\hat{\Omega}_e$$$.
  $$$\sum_e \int_{\Omega_e} f(\vec{x}) \;\text{d}\vec{x} =
        \sum_e \int_{\hat{\Omega}_e} f(\vec{\xi}) \left|\mathcal{J}_e\right| \;\text{d}\vec{\xi}$$$
- $$$\mathcal{J}_e$$$ is the Jacobian of the map from the physical element to the reference element.
- To approximate the reference element integrals numerically, we use quadrature (typically "Gaussian Quadrature"):
  $$$\sum_e \int_{\hat{\Omega}_e} f(\vec{\xi}) \left|\mathcal{J}_e\right| \;\text{d}\vec{\xi} \approx
        \sum_e \sum_{q} w_{q} f( \vec{x}_{q}) \left|\mathcal{J}_e(\vec{x}_{q})\right|$$$
- $$$\vec{x}_{q}$$$ is the spatial location of the $$$q$$$th quadrature point and $$$w_{q}$$$ is its associated weight.
- MOOSE handles multiplication by the Jacobian and the weight automatically, thus your `Kernel` is only responsible for computing the $$$f(\vec{x}_{q})$$$ part of the integrand.
- Under certain common situations, the quadrature approximation is exact!
    - For example, in 1 dimension, Gaussian Quadrature can exactly integrate polynomials of order $$$2n-1$$$ with $$$n$$$ quadrature points.
- Note that sampling $$$u_h$$$ at the quadrature points yields:
   $$$\begin{aligned}
    u(\vec{x}_{q}) &\approx u_h(\vec{x}_{q}) = \sum u_j \phi_j(\vec{x}_{q}) \\
    \nabla u (\vec{x}_{q}) &\approx \nabla u_h(\vec{x}_{q}) = \sum u_j \nabla \phi_j(\vec{x}_{q})\end{aligned}$$$
- And our weak form becomes:
  $$$\begin{aligned}
  R_i(u_h) &= \sum_e \sum_{q} w_{q} \left|\mathcal{J}_e\right|\left[ \nabla\psi_i\cdot k \nabla u_h + \psi_i \left(\vec\beta\cdot \nabla u_h \right) - \psi_i f \right](\vec{x}_{q}) \\
  &- \sum_f \sum_{q_{face}} w_{q_{face}} \left|\mathcal{J}_f\right| \left[\psi_i k \nabla u_h \cdot \vec{n} \right](\vec x_{q_{face}})
  \end{aligned}$$$
- The second sum is over boundary faces, $$$f$$$.
- MOOSE `Kernels` must provide each of the terms in square brackets (evaluated at $$$\vec{x}_{q}$$$ or $$$\vec x_{q_{face}}$$$ as necessary).

# Newton's Method

- We now have a nonlinear system of equations,
  $$$R_i(u_h)=0, \qquad i=1,\ldots, N$$$
  to solve for the coefficients $$$u_j, j=1,\dots,N$$$.

- Newton's method has good convergence properties, we use it to solve this system of nonlinear equations.
- Newton's method is a "root finding" method: it finds zeros of nonlinear equations.
- Newton's Method in "Update Form" for finding roots of the scalar equation
  $$$\begin{array}{rl}f(x)&=0, f(x): \mathbb{R} &\rightarrow \mathbb{R}\textrm{ is given by}:\\
  f'(x_n) \delta x_{n+1} &= -f(x_n) \\
  x_{n+1} &= x_n + \delta x_{n+1}\end{array}$$$
- We don't have just one scalar equation: we have a system of nonlinear equations.
- This leads to the following form of Newton's Method:

    $$$\begin{aligned}
    \mathbf{J}(\vec{u}_n) \delta\vec{u}_{n+1} &= -\vec{R}(\vec{u}_n) \\
    \vec{u}_{n+1} &= \vec{u}_n + \delta\vec{u}_{n+1}\end{aligned}$$$

- Where $$$\mathbf{J}(\vec{u}_n)$$$ is the Jacobian matrix evaluated at the current iterate:
    $$$J_{ij}(\vec{u}_n) = \frac{\partial R_i(\vec{u}_n)}{\partial u_j}$$$

- Note that:
    $$$\frac{\partial u_h}{\partial u_j} =
      \sum_k\frac{\partial }{\partial u_j}\left(u_k \phi_k\right) = \phi_j
    \qquad
    \frac{\partial \left(\nabla u_h\right)}{\partial u_j} =
      \sum_k \frac{\partial }{\partial u_j}\left(u_k \nabla \phi_k\right) = \nabla \phi_j$$$

# Newton for a Simple Equation

- Consider the convection-diffusion equation with nonlinear $$$k$$$, $$$\vec{\beta}$$$, and $$$f$$$:
    $$$\begin{aligned}- \nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u = f\end{aligned}$$$

- The $$$i^{th}$$$ component of the residual vector is:
    $$$\begin{aligned}
    R_i(u_h) = \left(\nabla\psi_i, k\nabla u_h \right) - \langle\psi_i, k\nabla u_h\cdot \hat{n} \rangle +
    \left(\psi_i, \vec{\beta} \cdot \nabla u_h\right) - \left(\psi_i, f\right)\end{aligned}$$$


- Using the previously-defined rules for $$$\frac{\partial u_h}{\partial u_j}$$$ and $$$\frac{\partial \left(\nabla u_h\right)}{\partial u_j}$$$, the $$$(i,j)$$$ entry of the Jacobian is then:

$$$\begin{aligned} J_{ij}(u_h) &= \left(\nabla\psi_i, \frac{\partial k}{\partial u_j}\nabla u_h \right) + \left(\nabla\psi_i, k \nabla \phi_j \right) - \left \langle\psi_i, \frac{\partial k}{\partial u_j}\nabla u_h\cdot \hat{n} \right\rangle \\&- \left \langle\psi_i, k\nabla \phi_j\cdot \hat{n} \right\rangle + \left(\psi_i, \frac{\partial \vec{\beta}}{\partial u_j} \cdot\nabla u_h\right) + \left(\psi_i, \vec{\beta} \cdot \nabla \phi_j\right) - \left(\psi_i, \frac{\partial f}{\partial u_j}\right)\end{aligned}$$$

- Note that even for this "simple" equation, the Jacobian entries are nontrivial: they depend on the partial derivatives of $$$k$$$, $$$\vec{\beta}$$$, and $$$f$$$, which may be difficult or time-consuming to compute analytically.

- In a multiphysics setting with many coupled equations and complicated material properties, the Jacobian might be extremely difficult to determine.

# Chain Rule

- On the previous slide, the term $$$\frac{\partial f}{\partial u_j}$$$ was used, where $$$f$$$ was a nonlinear forcing function.

- The chain rule allows us to write this term as

  $$$\begin{aligned}
    \frac{\partial f}{\partial u_j} &= \frac{\partial f}{\partial u_h} \frac{\partial u_h}{\partial u_j}
    \\
    &=\frac{\partial f}{\partial u_h} \phi_j\end{aligned}$$$

- If a functional form of $$$f$$$ is known, e.g. $$$f(u) = \sin(u)$$$, this
  formula implies that its Jacobian contribution is given by

   $$$\frac{\partial f}{\partial u_j} = \cos(u_h) \phi_j$$$

# Jacobian Free Newton Krylov

- $$$\mathbf{J}(\vec{u}_n)\delta \vec{u}_{n+1} = -\vec{R}(\vec{u}_n)$$$ is a linear system solved during each Newton step.
- For simplicity, we can write this linear system as $$$\mathbf{A}\vec{x} = \vec{b}$$$, where:
    - $$$\mathbf{A} \equiv \mathbf{J}(\vec{u}_n)$$$
    - $$$\vec{x} \equiv \delta \vec{u}_{n+1}$$$
    - $$$\vec{b} \equiv -\vec{R}(\vec{u}_n)$$$
- We employ an iterative Krylov method (e.g. GMRES) to produce a sequence of iterates $$$\vec{x}_k \rightarrow \vec{x}$$$, $$$k=1,2,\ldots$$$
- $$$\mathbf{A}$$$ and $$$\vec{b}$$$ remain *fixed* during the iterative process.
- The "linear residual" at step $$$k$$$ is defined as

  $$$\vec{\rho}_k \equiv \mathbf{A}\vec{x}_k - \vec{b}$$$

- MOOSE prints the norm of this vector, $$$\|\vec{\rho}_k\|$$$, at each iteration, if you set `print_linear_residuals = true` in the `Outputs` block.

- The "nonlinear residual" printed by MOOSE is $$$\|\vec{R}(\vec{u}_n)\|$$$.

- By iterate $$$k$$$, the Krylov method has constructed the subspace

  $$$\mathcal{K}_k = \text{span}\{ \vec{b}, \mathbf{A}\vec{b}, \mathbf{A}^2\vec{b}, \ldots, \mathbf{A}^{k-1}\vec{b}\}$$$

- Different Krylov methods produce the $$$\vec{x}_k$$$ iterates in different ways:
- Conjugate Gradients: $$$\vec{\rho}_k$$$ orthogonal to $$$\mathcal{K}_k$$$.
- GMRES/MINRES: $$$\vec{\rho}_k$$$ has minimum norm for $$$\vec{x}_k$$$ in $$$\mathcal{K}_k$$$.
- Biconjugate Gradients: $$$\vec{\rho}_k$$$ is orthogonal to $$$\mathcal{K}_k(\mathbf{A}^T)$$$

- $$$\mathbf{J}$$$ is never explicitly needed to construct the subspace, only the action of $$$\mathbf{J}$$$ on a vector is required.


- This action can be approximated by:
    $$$\mathbf{J}\vec{v} \approx \frac{\vec{R}(\vec{u} + \epsilon\vec{v}) - \vec{R}(\vec{u})}{\epsilon}$$$

- This form has many advantages:
    - No need to do analytic derivatives to form $$$\mathbf{J}$$$
    - No time needed to compute $$$\mathbf{J}$$$ (just residual computations)
    - No space needed to store $$$\mathbf{J}$$$

# Wrap Up

- The Finite Element Method is a way of numerically approximating the solution of PDEs.
- Just like polynomial fitting, FEM finds coefficients for basis functions.
- The "solution" is the combination of the coefficients and the basis functions, and the solution can be sampled anywhere in the domain.
- We compute integrals numerically using quadrature.
- Newton's Method provides a mechanism for solving a system of nonlinear equations.
- The Jacobian Free Newton Krylov (JFNK) method allows us to avoid explicitly forming the Jacobian matrix while still computing its "action".
