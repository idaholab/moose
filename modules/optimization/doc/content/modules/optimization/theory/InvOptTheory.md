# Inverse Optimization id=sec:invOpt

This page provides a practical overview of the theory and equations needed to solve inverse optimization problems for users/developers of the optimization module.  This overview focuses on optimization problems constrained by the solution to a partial differential equation, i.e. PDE constrained optimization, and how to efficiently compute the various gradients needed to accelerate the convergence of the optimization solver.  See citations throughout this overview for a more in depth and rigorous overview of inverse optimization.  This overview on inverse optimization in the optimization module is split up into the following sections:

- [PDE Constrained Optimization Theory:](#sec:invOptTheory) Derives the gradient of the objective from its total derivative, compares the tangent linear and adjoint methods for computing it, and relates the adjoint method to the Lagrange multiplier derivation.
- [Steady State Heat Conduction:](#sec:adjoint) The heat conduction problem, its discrete residual, and the resulting adjoint problem and boundary conditions.
- [Parameter Derivatives of PDE:](#sec:PDEDerivs) Derivatives of the PDE with respect to the optimization parameters for [force inversion](#sec:forceInv), [convective boundary conditions](#sec:robinInv), and [thermal conductivity](#sec:material_inversion).  Force inversion examples, where PDE constrained optimization is used to parameterize boundary conditions and body loads, are provided on the [Force Inversion Examples page](forceInv_main.md).  Material inversion examples, where PDE constrained optimization is used to control material properties, are provided on the [Material Inversion Examples page](materialInv_main.md).

The overall flow of the optimization algorithm implemented in the MOOSE optimization module is shown in [fig:optCycle].  In this example, the internal heat source, $q_v$, is being parameterized to match the simulated and experimental steady state temperature fields, $T$ and $\widetilde{T}$, respectively.  Step one of the optimization cycle consists of adjusting the internal heat source, $q_v$.  In step two, the physics model is solved with the current $q_v$ to obtain a simulated temperature field, $T$.  Step two is often referred to as the forward solve.  In step three, the simulated and experimental temperature fields are compared via the objective function, $f$.  If $f$ is below the user defined threshold, the optimization cycle stops and the best fit parameterization of $q_v$ is found.  If $f$ is above the user defined threshold, the optimization algorithm determines a new $q_v$ and the process is repeated.  In the next section, methods for determining the next iteration of the parameterized value, in this case $q_v$, will be presented.  Problems solved using the optimization module are available on the [Examples page](optimization/examples/index.md).

!media large_media/optimization/fig_optCycle.png
       style=width:80%;margin:auto;padding-top:2.5%;
       id=fig:optCycle
       caption=Optimization cycle example for parameterizing an internal heat source distribution $q_v$ to match the simulated and experimental temperature field, $T$ and $\widetilde{T}$, respectively.

[comment0]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Constrained Inverse Optimization id=sec:invOptTheory

Inverse optimization is a mathematical framework to infer model parameters by minimizing the misfit between the experimental and simulation observables.  In the optimization module, our model is a PDE describing the physics of the experiment.  We solve our physics PDE using the finite element method as implemented in MOOSE.  The physics of our problem constrains our optimization algorithm.  A PDE-constrained inverse optimization framework is formulated as an abstract optimization problem [!citet](biegler2003large):
\begin{equation}\label{eq:optimization}
   \min_{\mathbf{p}} f\left(\mathbf{u},\mathbf{p}\right);\quad~\textrm{subject to}~\mathcal{R}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0} ,
\end{equation}
where $f(\mathbf{u},\mathbf{p})$ is our objective function which provides a scalar measure of the misfit between experimental and simulated responses, along with any regularization [!citet](neumaier1998solving).  The constraint, $\mathcal{R}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0}$, is the residual vector for the PDEs governing the multiphysics phenomena simulated by MOOSE (e.g. coupled heat and elasticity equations), the vector $\mathbf{p}$ contains design variables (e.g. material properties or loads) and the vector $\mathbf{u}$ contains state variables (e.g. temperature and displacement fields).  [eq:optimization] appears simple on the outset but is extremely difficult to solve. The solution space can span millions of degrees of freedom and the parameter space can also be very large. Finally, the PDEs can be highly nonlinear, time-dependent and tightly coupling complex phenomena across multiple physics.

Optimization problems can be solved using either global (gradient-free) or local (gradient-based) approaches [!citet](aster2018parameter). Global approaches require a large number of iterations compared to gradient-based approaches (e.g. conjugate gradient or Newton-type methods), making the latter more suitable to problems with a large parameter space and computationally expensive models.  The [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) optimization library [!citet](balay2019petsc) is used to solve [eq:optimization].  Optimization libraries like TAO require access to functions for computing the objective ($f$), gradient $\left(\mathrm{d}f/\mathrm{d}\mathbf{p}\right)$ and Hessian $\left(\mathrm{d}^2f/\mathrm{d}\mathbf{p}^2\right)$ or a function to take the action of the Hessian on a vector.  An objective function measuring the misfit or distance between the simulation and experimental data usually has the form
\begin{equation}\label{eq:objective}
f(\mathbf{u}, \mathbf{p}) = \frac{1}{2}\sum^N_i \left(u_i - \tilde u_i\right)^2 + \frac{\rho}{2}\sum^{n_p}_j p_jp_j ,
\end{equation}
where the first summation over $(i=1..N)$ is an L$_2$ norm or euclidean distance between experimental measurement points, $\mathbf{\tilde  u}$, and the simulated solution, $\mathbf{u}$.  The second summation over $(j=1..n_p)$ provides Tikhonov regularization of the design variables, $\mathbf{p}$, for ill-posed problems where $\rho$ controls the amount of regularization.  Other types of regularization may also be used.

Gradient-free optimization solvers only require a function to solve for the objective given in [eq:objective].  Solving for the objective only requires solving a forward problem to determine $\mathbf{u}$ and then plugging that into [eq:objective] to determine $f$.  The forward problem is defined as the FEM model of the experiment which the analyst should have already made before attempting to perform optimization.  The parameters that go into the forward problem (e.g. pressure distributions on sidesets or material properties) are adjusted by the optimization solver and the forward problem is recomputed.  This process continues until $f$ is below some user defined threshold. The basic gradient-free solver available in TAO is the simplex or Nelder-Mead method.  Gradient-free optimization solvers are robust and straight-forward to use.  Unfortunately, their computational cost scales exponentially with the number of parameters.  When the forward model is a computationally expensive FEM model, gradient-free approaches quickly become computationally expensive.

Gradient-based optimization algorithms require fewer iterations but require functions to solve for the gradient vector and sometimes Hessians matrix.  TAO has `petsc_options` to evaluate finite difference based gradients and Hessians by solving the objective function multiple times with perturbed parameters, which also requires multiple solves of the forward problem.  Finite difference gradients and Hessians are good for testing an optimization approach but become too expensive for realistic problems.

Given the large parameter space, we resort to the adjoint method for gradient computation; unlike finite difference approaches, the computational cost of adjoint methods is independent of the number of parameters [!citep](plessix2006review).  The derivation below introduces the adjoint method from the total derivative of the objective, following [!citet](martins2013review), Section 4.1 of [!citet](kenway2019effective), and the dolfin-adjoint lecture by [!citet](funke2017dolfinadjoint).  The size of each derivative is written beneath it, where $n_u$ is the number of degrees of freedom in the discretized state vector $\mathbf{u}$ and $n_p$ is the number of design parameters in $\mathbf{p}$.  The residual vector $\mathcal{R}$ has one entry per degree of freedom, $\mathcal{R}\in\mathbb{R}^{n_u}$, and the objective $f$ is a scalar.

### Total Derivative of the Objective id=sec:totalDerivative

The forward solve defines the state as an implicit function of the parameters, $\mathbf{u}=\mathbf{u}(\mathbf{p})$, so the objective depends on $\mathbf{p}$ both directly and through $\mathbf{u}$.  Applying the chain rule gives the gradient,
\begin{equation}\label{eq:objectiveGrad}
\underset{1\times n_p}{\frac{\mathrm{d}f}{\mathrm{d}\mathbf{p}}} =
\underset{1\times n_p}{\frac{\partial f}{\partial\mathbf{p}}} +
\underset{1\times n_u}{\frac{\partial f}{\partial\mathbf{u}}}\,
\underset{n_u\times n_p}{\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}\mathbf{p}}} .
\end{equation}
The partial derivatives $\partial f/\partial\mathbf{p}$ and $\partial f/\partial\mathbf{u}$ are explicit and cheap to evaluate.  For [eq:objective], $\partial f/\partial\mathbf{p}=\rho\,\mathbf{p}^\top$ is the regularization term and $\partial f/\partial\mathbf{u}$ is the misfit $u_i-\tilde u_i$ at the measurement points, mapped onto the degrees of freedom by the shape functions evaluated at those points.  The total derivative $\mathrm{d}\mathbf{u}/\mathrm{d}\mathbf{p}$ is expensive because $\mathbf{u}$ is only known implicitly through $\mathcal{R}(\mathbf{u},\mathbf{p})=\mathbf{0}$.

The residual must remain zero as the parameters change, so its total derivative with respect to $\mathbf{p}$ also vanishes.  Applying the chain rule to $\mathcal{R}(\mathbf{u}(\mathbf{p}),\mathbf{p})$ gives
\begin{equation}\label{eq:gradZero}
\underset{n_u\times n_p}{\frac{\mathrm{d}\mathcal{R}}{\mathrm{d}\mathbf{p}}} =
\underset{n_u\times n_p}{\frac{\partial\mathcal{R}}{\partial\mathbf{p}}} +
\underset{n_u\times n_u}{\frac{\partial\mathcal{R}}{\partial\mathbf{u}}}\,
\underset{n_u\times n_p}{\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}\mathbf{p}}} = \mathbf{0} .
\end{equation}
The matrix $\partial\mathcal{R}/\partial\mathbf{u}$ is the Jacobian of the forward problem, the same matrix used in its Newton iterations.  Moving $\partial\mathcal{R}/\partial\mathbf{p}$ in [eq:gradZero] to the right-hand side gives a linear system for $\mathrm{d}\mathbf{u}/\mathrm{d}\mathbf{p}$,
\begin{equation}\label{eq:gradZeroArrange}
\underset{n_u\times n_u}{\frac{\partial\mathcal{R}}{\partial\mathbf{u}}}\,
\underset{n_u\times n_p}{\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}\mathbf{p}}} =
-\underset{n_u\times n_p}{\frac{\partial\mathcal{R}}{\partial\mathbf{p}}} ,
\end{equation}
and multiplying [eq:gradZeroArrange] on the left by the inverse Jacobian gives
\begin{equation}\label{eq:dudp}
\underset{n_u\times n_p}{\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}\mathbf{p}}} =
-\underset{n_u\times n_u}{\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^{-1}}\,
\underset{n_u\times n_p}{\frac{\partial\mathcal{R}}{\partial\mathbf{p}}} .
\end{equation}
Substituting [eq:dudp] into [eq:objectiveGrad] gives the gradient in terms of quantities that can be computed:
\begin{equation}\label{eq:objectiveGrad2}
\underset{1\times n_p}{\frac{\mathrm{d}f}{\mathrm{d}\mathbf{p}}} =
\underset{1\times n_p}{\frac{\partial f}{\partial\mathbf{p}}} -
\underset{1\times n_u}{\frac{\partial f}{\partial\mathbf{u}}}\,
\underset{n_u\times n_u}{\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^{-1}}\,
\underset{n_u\times n_p}{\frac{\partial\mathcal{R}}{\partial\mathbf{p}}} .
\end{equation}
The inverse Jacobian is never formed explicitly.  The triple product in [eq:objectiveGrad2] is associative, so it can be evaluated from the right or from the left, and each order replaces the inverse with a different linear solve.  Evaluating from the right gives the tangent linear (direct) method; evaluating from the left gives the adjoint method.

#### Tangent Linear Method

The tangent linear method groups the last two factors of [eq:objectiveGrad2], $\left(\partial\mathcal{R}/\partial\mathbf{u}\right)^{-1}\partial\mathcal{R}/\partial\mathbf{p}$, which by [eq:dudp] equals $-\mathrm{d}\mathbf{u}/\mathrm{d}\mathbf{p}$.  Instead of inverting the Jacobian, [eq:gradZeroArrange] is solved directly.  Writing its $n_p$ columns side by side shows that each parameter adds one right-hand side:
\begin{equation}\label{eq:tangentLinearBlock}
\frac{\partial\mathcal{R}}{\partial\mathbf{u}}
\begin{bmatrix}
\dfrac{\mathrm{d}\mathbf{u}}{\mathrm{d}p_1} & \dfrac{\mathrm{d}\mathbf{u}}{\mathrm{d}p_2} & \cdots & \dfrac{\mathrm{d}\mathbf{u}}{\mathrm{d}p_{n_p}}
\end{bmatrix}
= -
\begin{bmatrix}
\dfrac{\partial\mathcal{R}}{\partial p_1} & \dfrac{\partial\mathcal{R}}{\partial p_2} & \cdots & \dfrac{\partial\mathcal{R}}{\partial p_{n_p}}
\end{bmatrix} .
\end{equation}
Column $j$ of [eq:tangentLinearBlock] is the tangent linear equation for parameter $p_j$,
\begin{equation}\label{eq:tangentLinear}
\underset{n_u\times n_u}{\frac{\partial\mathcal{R}}{\partial\mathbf{u}}}\,
\underset{n_u\times 1}{\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}p_j}} =
-\underset{n_u\times 1}{\frac{\partial\mathcal{R}}{\partial p_j}} ,
\qquad j=1,\dots,n_p .
\end{equation}
Its solution $\mathrm{d}\mathbf{u}/\mathrm{d}p_j$ is the linearized response of the whole state field to a unit change in $p_j$.  Substituting each solution into column $j$ of [eq:objectiveGrad] gives one entry of the gradient,
\begin{equation}\label{eq:tangentLinearGrad}
\underset{1\times 1}{\frac{\mathrm{d}f}{\mathrm{d}p_j}} =
\underset{1\times 1}{\frac{\partial f}{\partial p_j}} +
\underset{1\times n_u}{\frac{\partial f}{\partial\mathbf{u}}}\,
\underset{n_u\times 1}{\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}p_j}} .
\end{equation}
The tangent linear method needs $n_p$ solves of [eq:tangentLinear], one per parameter, all with the forward Jacobian.  Its cost grows with the number of parameters but does not depend on the number of objectives, because [eq:tangentLinear] does not involve $f$.

#### Adjoint Method

The adjoint method groups the first two factors of [eq:objectiveGrad2] and names their product the adjoint variable,
\begin{equation}\label{eq:lambdaDef}
\underset{1\times n_u}{\lambda^\top} \equiv
\underset{1\times n_u}{\frac{\partial f}{\partial\mathbf{u}}}\,
\underset{n_u\times n_u}{\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^{-1}} .
\end{equation}
To remove the inverse, multiply both sides of [eq:lambdaDef] on the right by $\partial\mathcal{R}/\partial\mathbf{u}$.  Since $\left(\partial\mathcal{R}/\partial\mathbf{u}\right)^{-1}\partial\mathcal{R}/\partial\mathbf{u}=\mathbf{I}$, this gives
\begin{equation}\label{eq:adjointRow}
\underset{1\times n_u}{\lambda^\top}\,
\underset{n_u\times n_u}{\frac{\partial\mathcal{R}}{\partial\mathbf{u}}} =
\underset{1\times n_u}{\frac{\partial f}{\partial\mathbf{u}}} .
\end{equation}
[eq:adjointRow] is a linear system for the row vector $\lambda^\top$.  Transposing both sides with $(\mathbf{A}\mathbf{B})^\top=\mathbf{B}^\top\mathbf{A}^\top$ puts it in the standard column form, the adjoint equation:
\begin{equation}\label{eq:adjoint}
\underset{n_u\times n_u}{\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^\top}\,
\underset{n_u\times 1}{\lambda} =
\underset{n_u\times 1}{\left(\frac{\partial f}{\partial\mathbf{u}}\right)^\top} .
\end{equation}
Replacing the first two factors of [eq:objectiveGrad2] with $\lambda^\top$ from [eq:lambdaDef] gives the gradient,
\begin{equation}\label{eq:adjointGrad}
\underset{1\times n_p}{\frac{\mathrm{d}f}{\mathrm{d}\mathbf{p}}} =
\underset{1\times n_p}{\frac{\partial f}{\partial\mathbf{p}}} -
\underset{1\times n_u}{\lambda^\top}\,
\underset{n_u\times n_p}{\frac{\partial\mathcal{R}}{\partial\mathbf{p}}} .
\end{equation}
The parameters do not appear in [eq:adjoint], so one adjoint solve per objective gives the gradient with respect to all $n_p$ parameters.  [tab:directVsAdjoint] compares the two methods.

!table id=tab:directVsAdjoint caption=Cost of the tangent linear and adjoint methods for one objective and $n_p$ parameters.
| | Tangent linear method | Adjoint method |
| :- | :- | :- |
| Linear system | [eq:tangentLinear]: $\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}p_j}=-\frac{\partial\mathcal{R}}{\partial p_j}$ | [eq:adjoint]: $\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^\top\lambda=\left(\frac{\partial f}{\partial\mathbf{u}}\right)^\top$ |
| Matrix | $\frac{\partial\mathcal{R}}{\partial\mathbf{u}}$ | $\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^\top$ |
| Gradient | [eq:tangentLinearGrad] | [eq:adjointGrad] |
| Linear solves | $n_p$, one per parameter | 1 per objective |

For a transient problem with $n_t$ time steps, $\mathbf{u}$ and $\mathcal{R}$ stack the states and residuals of every step, so $\partial\mathcal{R}/\partial\mathbf{u}$ is the Jacobian of the entire time history, not of a single step.  Each step depends only on earlier steps, so this matrix is block lower triangular, with the single-step Jacobians on its diagonal.  The tangent linear method solves it by forward substitution, one step at a time alongside the forward solve, for a cost of $n_p\,n_t$ single-step linear solves and no stored history.  The transpose in the adjoint method is block upper triangular, so it is solved by backward substitution from the final time to the initial time, for a cost of $n_t$ single-step linear solves.  Each backward step needs the forward solution at that step, so the forward history must be stored or recomputed, as [TransientAndAdjoint.md] does.  Both methods remain possible for transient problems; the adjoint is still cheaper for many parameters but requires this storage.

The adjoint system has the same size and sparsity as the forward Jacobian, so it costs about as much as one Newton step of the forward problem.  It is linear even when the forward problem is nonlinear, with the Jacobian evaluated at the converged forward solution.  For a symmetric operator such as steady heat conduction, $\left(\partial\mathcal{R}/\partial\mathbf{u}\right)^\top=\partial\mathcal{R}/\partial\mathbf{u}$, and the adjoint problem is the forward operator loaded by the misfit at the measurement points.  The gradient is only as accurate as the Jacobian, so [SteadyAndAdjoint.md] requires the forward kernels to provide an exact Jacobian.

In summary, the adjoint gradient computation consists of four steps:

1. Solve the forward problem $\mathcal{R}(\mathbf{u},\mathbf{p})=\mathbf{0}$ for $\mathbf{u}$.
1. Evaluate the objective $f$ and the adjoint load $\left(\partial f/\partial\mathbf{u}\right)^\top$, i.e. the misfit at the measurement points.
1. Solve the adjoint equation [eq:adjoint] for $\lambda$.
1. Evaluate $\lambda^\top\partial\mathcal{R}/\partial\mathbf{p}$ and $\partial f/\partial\mathbf{p}$ and assemble the gradient with [eq:adjointGrad].

### Comparison with the Lagrange Multiplier Derivation id=sec:lagrangeComparison

The same equations follow from a Lagrangian [!citep](bradley2013pde,walsh2013source), where the adjoint variable plays the role of a Lagrange multiplier enforcing the PDE constraint:
\begin{equation}\label{eq:discreteLagrangian}
\mathcal{L}(\mathbf{u},\mathbf{p},\lambda) = f(\mathbf{u},\mathbf{p}) - \lambda^\top\mathcal{R}(\mathbf{u},\mathbf{p}) .
\end{equation}
At the forward solution $\mathcal{R}=\mathbf{0}$, so $\mathcal{L}=f$ for any $\lambda$ and $\mathrm{d}f/\mathrm{d}\mathbf{p}=\mathrm{d}\mathcal{L}/\mathrm{d}\mathbf{p}$:
\begin{equation}\label{eq:lagrangianGrad}
\frac{\mathrm{d}\mathcal{L}}{\mathrm{d}\mathbf{p}} =
\frac{\partial f}{\partial\mathbf{p}} - \lambda^\top\frac{\partial\mathcal{R}}{\partial\mathbf{p}} +
\left(\frac{\partial f}{\partial\mathbf{u}} - \lambda^\top\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)\frac{\mathrm{d}\mathbf{u}}{\mathrm{d}\mathbf{p}} .
\end{equation}
Since $\lambda$ is arbitrary, we can choose it so that the term in parentheses in [eq:lagrangianGrad] vanishes, which removes the expensive $\mathrm{d}\mathbf{u}/\mathrm{d}\mathbf{p}$.  Setting that term to zero is [eq:adjointRow], and transposing it gives the adjoint equation [eq:adjoint].  The terms that remain in [eq:lagrangianGrad] are the gradient [eq:adjointGrad].  Equivalently, requiring $\mathcal{L}$ to be stationary with respect to $\lambda$, $\mathbf{u}$, and $\mathbf{p}$ gives the forward problem, the adjoint problem, and the first-order optimality condition $\mathrm{d}f/\mathrm{d}\mathbf{p}=\mathbf{0}$.  The minus sign in [eq:discreteLagrangian] is a convention.  Choosing a plus sign flips the sign of $\lambda$ and of the $\lambda^\top\partial\mathcal{R}/\partial\mathbf{p}$ term in [eq:adjointGrad], but leaves the gradient unchanged.

The two derivations give the same result.  The total derivative shows where the efficiency comes from: the tangent linear and adjoint methods evaluate the same matrix product, [eq:objectiveGrad2], in opposite orders.  The Lagrangian is convenient when working with the continuous PDE, because integrating the constraint term by parts directly yields the adjoint PDE and its boundary conditions.  The [next section](#sec:adjoint) instead works with the discrete residual, which matches the MOOSE implementation.  Transposing the discrete forward Jacobian, as [SteadyAndAdjoint.md] does, gives the exact gradient of the discretized objective.  Discretizing the continuous adjoint PDE gives the same result for self-adjoint operators with a Galerkin discretization, such as steady heat conduction; in general the two differ by discretization error.

[comment1]: <> (% ----------------------------------------------------------------------------------------------------------%)

## Steady State Heat Conduction id=sec:adjoint

The rest of this page applies the adjoint method to steady state heat conduction.  The PDE-constrained optimization problem from [eq:optimization] is
\begin{equation} \label{eqn:optimization_problem}
	\begin{aligned}
	& \min_{\mathbf{p}}
	& & f\left(T,\mathbf{p}\right) = \frac{1}{2} \sum_{i=1}^{N} \left( T_i - \widetilde {T}_i \right)^2 , \\
	& \text{subject to}
	& & \nabla \cdot \kappa \nabla T + g_b =0, & \text{in}~\Omega , \\
\end{aligned}
\end{equation}
where $f$ is the objective function from [eq:objective] without regularization, $g_b$ is the body heat source, and $\widetilde{T}_i$ is the measured temperature compared with the simulated temperature $T_i$ at measurement location $x_i$.  Other forms for the objective function are possible such as different norms or different types of measurements that may require integration over a line or volume.

The PDE has the boundary conditions
\begin{equation} \label{eqn:optimization_bcs}
	\begin{aligned}
	& & & T = T_D, &\text{on}~\Gamma_D, \\
	& & & \left( \kappa \nabla T \right) \cdot \boldsymbol{n} = G\left(T\right), &\text{on}~\Gamma_R , \\
\end{aligned}
\end{equation}
where $\boldsymbol{n}$ is the outward normal vector, $\Gamma_D$ is the Dirichlet boundary, $\Gamma_R$ is the Robin or mixed boundary, and $G$ is the heat flux into the body.  Common cases for $G\left(T\right)$ are:
\begin{equation}
\begin{aligned}\label{eq:robin_bc_types}
   &\text{Neumann:     }~G\left(T\right) = g_n , \\
   &\text{Convection:  }~G\left(T\right) = h (T_{\infty} - T) , \\
\end{aligned}
\end{equation}
where $g_n$ is a prescribed flux, $h$ is the heat transfer coefficient, and $T_\infty$ is the ambient temperature.  For $N$ point measurements, the objective function can also be written as an integral,
\begin{equation}  \label{eqn:objective_integral}
	f = \frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \widetilde{T}_i \right)^2\text{d}\Omega.
\end{equation}

### Discrete Residual id=sec:discreteResidual

The state vector $\mathbf{u}$ holds the nodal temperatures, so $T(x)\approx\sum_\beta\phi_\beta(x)u_\beta$, where $\phi_\beta$ is the continuous Lagrange shape function of node $\beta$.  With the Galerkin method, the residual for the test function $\phi_\alpha$ is
\begin{equation}\label{eq:discretePDE}
\mathcal{R}_\alpha = \int_\Omega\nabla\phi_\alpha\cdot\kappa\nabla T~\text{d}\Omega - \int_\Omega\phi_\alpha g_b~\text{d}\Omega - \int_{\Gamma_R}\phi_\alpha G~\text{d}\Gamma .
\end{equation}
Rows of $\mathcal{R}$ for nodes on $\Gamma_D$ are replaced by $u_\alpha-T_D$, which is how MOOSE imposes Dirichlet conditions.  When $\kappa$ does not depend on $T$, [eq:discretePDE] is $\mathcal{R}=\mathbf{K}\mathbf{u}-\mathbf{g}$, with
\begin{equation} \label{eq:elementwise_discretized_terms}
\begin{aligned}
    K_{\alpha\beta}&=\int_\Omega\nabla\phi_\alpha\cdot\kappa\nabla\phi_\beta~\text{d}\Omega + \int_{\Gamma_R}\phi_\alpha h\,\phi_\beta~\text{d}\Gamma,\\
    g_\alpha & = \int_\Omega \phi_\alpha g_b~\text{d}\Omega + \int_{\Gamma_R} \phi_\alpha \left(g_n + hT_\infty\right)~\text{d}\Gamma ,
\end{aligned}
\end{equation}
where $\mathbf{K}=\partial\mathcal{R}/\partial\mathbf{u}$ is the symmetric stiffness matrix and $\mathbf{g}$ is the load vector.  The convection term splits between them: $hT$ multiplies the state and goes into $\mathbf{K}$, while $hT_\infty$ is a load and goes into $\mathbf{g}$.

### Adjoint Problem id=sec:adjointProblem

The adjoint equation [eq:adjoint] for this residual is $\mathbf{K}^\top\lambda=\left(\partial f/\partial\mathbf{u}\right)^\top$.  The heat conduction operator is symmetric, so $\mathbf{K}^\top=\mathbf{K}$ and the adjoint problem uses the same conduction kernels as the forward problem.  Its only load is the derivative of [eqn:objective_integral], $\partial f/\partial u_\alpha=\sum_i(T_i-\widetilde{T}_i)\,\phi_\alpha(x_i)$, which is a point source at each measurement location with magnitude equal to the misfit.

The adjoint boundary conditions follow from one rule.  The adjoint operator is the Jacobian $\partial\mathcal{R}/\partial\mathbf{u}$, so a term of [eq:discretePDE] appears in the adjoint problem only if it depends on the temperature.  Prescribed data, such as $g_b$, $g_n$, $T_\infty$, and $T_D$, are constant in $\mathbf{u}$.  Their derivatives vanish, so they do not appear in the adjoint problem.  Terms that multiply the temperature, such as $hT$ in the convective boundary condition, remain with $T$ replaced by $\lambda$.  [tab:adjointBCs] applies this rule to each forward boundary condition.

!table id=tab:adjointBCs caption=Adjoint boundary conditions for steady state heat conduction.  Only the parts of each forward boundary term that depend on $T$ remain in the adjoint problem.
| Forward boundary condition | Term in $\mathcal{R}$ | Depends on $T$ | Adjoint boundary condition |
| :- | :- | :- | :- |
| Dirichlet, $T=T_D$ | $u_\alpha-T_D$ | $u_\alpha$ | $\lambda=0$ |
| Neumann, $\kappa\nabla T\cdot\boldsymbol{n}=g_n$ | $-\int_{\Gamma_R}\phi_\alpha g_n~\text{d}\Gamma$ | none | $\kappa\nabla\lambda\cdot\boldsymbol{n}=0$ |
| Convection, $\kappa\nabla T\cdot\boldsymbol{n}=h(T_\infty-T)$ | $\int_{\Gamma_R}\phi_\alpha h(T-T_\infty)~\text{d}\Gamma$ | $hT$ | $\kappa\nabla\lambda\cdot\boldsymbol{n}=-h\lambda$ |

The Dirichlet condition needs one more step.  The Dirichlet row $u_\alpha-T_D$ has unit derivative with respect to $u_\alpha$ and does not depend on $\mathbf{p}$, so it contributes nothing to $\lambda^\top\partial\mathcal{R}/\partial\mathbf{p}$.  The integrals in [#sec:PDEDerivs] are evaluated over the whole adjoint field, including the Dirichlet nodes, so they equal $\lambda^\top\partial\mathcal{R}/\partial\mathbf{p}$ only when $\lambda=0$ at those nodes.  [SteadyAndAdjoint.md] enforces this by zeroing the Dirichlet rows and columns of the adjoint system.

The forward data affect $\lambda$ only through the forward temperature $T$, which sets the misfit and, for nonlinear problems, the Jacobian.  In strong form, the adjoint problem is
\begin{equation} \label{eqn:adjoint_problem}
\begin{aligned}
\nabla\cdot \kappa \nabla \lambda +  \sum_{i=1}^{N} \delta (x - x_i)(T_i - \widetilde{T}_i) &=0, ~\text{in}~\Omega,\\
\lambda  &= 0, ~\text{on}~\Gamma_D, \\
\kappa \nabla \lambda \cdot \boldsymbol{n} & = -h\lambda, ~\text{on}~{\Gamma_R}, \\
\end{aligned}
\end{equation}
with $h=0$ on Neumann boundaries.  When the adjoint is solved as a separate sub-app rather than with [SteadyAndAdjoint.md], the input file is the forward input with the body loads and Neumann fluxes removed, the Dirichlet values set to zero, convective boundary conditions kept with $T_\infty=0$, and a [ReporterPointSource.md] applying the misfit.

[comment2]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivatives for Inversion id=sec:PDEDerivs

The last term needed for the gradient [eq:adjointGrad] is $\lambda^\top\partial\mathcal{R}/\partial\mathbf{p}$.  The adjoint field is interpolated with the same shape functions as the temperature, $\lambda(x)=\sum_\alpha\phi_\alpha(x)\lambda_\alpha$.  Multiplying $\partial\mathcal{R}_\alpha/\partial p_j$ by $\lambda_\alpha$ and summing over $\alpha$ therefore replaces $\phi_\alpha$ with $\lambda$, and $\nabla\phi_\alpha$ with $\nabla\lambda$, inside each integral of [eq:discretePDE].  The result is an integral of known fields that is evaluated at quadrature points without forming the matrix $\partial\mathcal{R}/\partial\mathbf{p}$.

The derivatives are grouped by which term of [eq:discretePDE] contains the parameters:

- +Force inversion+ ([#sec:forceInv]): the parameters appear only in the load $g_b$ or $g_n$.  The stiffness matrix does not change with $\mathbf{p}$, so if the load is linear in $\mathbf{p}$, the temperature is too.  The misfit $\frac{1}{2}\sum_i(T_i-\widetilde{T}_i)^2$ is then a quadratic function of $\mathbf{p}$ with a single minimum, and a gradient-based solver reaches it from any initial guess.
- +Material inversion+ ([#sec:robinInv], [#sec:material_inversion]): the parameters appear in a coefficient that multiplies $T$, the convection coefficient $h$ or the conductivity $\kappa$.  The temperature depends nonlinearly on these parameters, the objective can have several local minima, and convergence depends on the initial guess and parameter bounds.

### Force Inversion id=sec:forceInv

For a parameterized body load $g_b(x;\mathbf{p})$, only the source term of [eq:discretePDE] depends on $\mathbf{p}$:
\begin{equation}\label{eq:bodyLoads}
\frac{\partial\mathcal{R}_\alpha}{\partial p_j} = -\int_\Omega \phi_\alpha\frac{\partial g_b}{\partial p_j}~\text{d}\Omega,
\qquad
\lambda^\top\frac{\partial\mathcal{R}}{\partial p_j} = -\int_\Omega \lambda\frac{\partial g_b}{\partial p_j}~\text{d}\Omega .
\end{equation}
Substituting into [eq:adjointGrad] gives $\mathrm{d}f/\mathrm{d}p_j=\int_\Omega\lambda\,\partial g_b/\partial p_j~\text{d}\Omega$ without regularization, which is what [ElementOptimizationSourceFunctionInnerProduct.md] computes.

For a volumetric heat source that varies linearly across the body,
\begin{equation}
g_b(x)=p_0+p_1x,
\end{equation}
with design parameters $p_0$ and $p_1$, [eq:bodyLoads] gives
\begin{equation}\label{eq:bodyLoad_example}
\lambda^\top\frac{\partial\mathcal{R}}{\partial\mathbf{p}} =
-\left[\int_\Omega\lambda~\text{d}\Omega ,\ \int_\Omega\lambda\,x~\text{d}\Omega\right] .
\end{equation}
An example using the derivative given in [eq:bodyLoad_example] is given in the [Distributed Body Load Example](forceInv_BodyLoad.md).

For $M$ point heat sources at known locations $x_j$ with unknown intensities $p_j$,
\begin{equation}
g_b(x)=\sum_{j=1}^{M}\delta\left(x-x_j \right)p_j ,
\end{equation}
the delta function evaluates the integrand of [eq:bodyLoads] at the source location:
\begin{equation}\label{eq:pointLoad}
\frac{\partial\mathcal{R}_\alpha}{\partial p_j} = -\phi_\alpha(x_j),
\qquad
\lambda^\top\frac{\partial\mathcal{R}}{\partial p_j} = -\lambda(x_j) .
\end{equation}
The gradient with respect to each point source intensity is the adjoint field sampled at that source.  This PDE derivative is used in the [Point Loads Example](forceInv_pointLoads.md).

For a parameterized Neumann flux $g_n(x;\mathbf{p})$ on $\Gamma_R$, the same derivation applies to the boundary term of [eq:discretePDE]:
\begin{equation}\label{eq:bcLoad}
\frac{\partial\mathcal{R}_\alpha}{\partial p_j} = -\int_{\Gamma_R} \phi_\alpha\frac{\partial g_n}{\partial p_j}~\text{d}\Gamma,
\qquad
\lambda^\top\frac{\partial\mathcal{R}}{\partial p_j} = -\int_{\Gamma_R} \lambda\frac{\partial g_n}{\partial p_j}~\text{d}\Gamma ,
\end{equation}
which [SideOptimizationNeumannFunctionInnerProduct.md] computes.  For a linearly varying heat flux
\begin{equation}
g_n(x)=p_0 + p_1x,
\end{equation}
with design parameters $p_0$ and $p_1$, [eq:bcLoad] gives
\begin{equation}\label{eq:neumannBC}
\lambda^\top\frac{\partial\mathcal{R}}{\partial\mathbf{p}} =
-\left[\int_{\Gamma_R}\lambda~\text{d}\Gamma ,\ \int_{\Gamma_R}\lambda\,x~\text{d}\Gamma\right] .
\end{equation}
The derivative given in [eq:neumannBC] is used in the [Neumann Boundary Condition Example](forceInv_NeumannBC.md).

A load that is nonlinear in its parameters makes even force inversion nonlinear.  For example, a Gaussian body source
\begin{equation}\label{eq:gaussOpt}
g_b(x) = a \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}
\end{equation}
has peak intensity $a$, peak location $b$, and width $c$.  The source is linear in $a$, so the derivative
\begin{equation}\label{eq:gaussOpt_a}
\lambda^\top\frac{\partial\mathcal{R}}{\partial a}
= -\int_\Omega \lambda \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}~\text{d}\Omega
\end{equation}
does not depend on $a$, and fitting $a$ alone has a single minimum.  The source is nonlinear in $b$, so the derivative
\begin{equation}\label{eq:gaussOpt_b}
\lambda^\top\frac{\partial\mathcal{R}}{\partial b}
= -\int_\Omega \lambda \frac{a(x-b)}{c^2} \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}~\text{d}\Omega
\end{equation}
depends on $b$.  Fitting the peak location is a nonlinear problem whose convergence depends on the initial guess for $b$ and on the measurement locations.

### Convective Boundary Conditions id=sec:robinInv

For the convection coefficient $h$ in [eq:robin_bc_types], the boundary term of [eq:discretePDE] is $\int_{\Gamma_R}\phi_\alpha h\left(T-T_\infty\right)\text{d}\Gamma$, so
\begin{equation}\label{eq:convectiveBC}
\frac{\partial\mathcal{R}_\alpha}{\partial h} = \int_{\Gamma_R} \phi_\alpha \left(T-T_{\infty} \right)~\text{d}\Gamma,
\qquad
\lambda^\top\frac{\partial\mathcal{R}}{\partial h} = \int_{\Gamma_R} \lambda \left(T-T_{\infty} \right)~\text{d}\Gamma .
\end{equation}
Substituting into [eq:adjointGrad] gives $\mathrm{d}f/\mathrm{d}h=\int_{\Gamma_R}\lambda\left(T_\infty-T\right)~\text{d}\Gamma$ without regularization.  The derivative contains the forward temperature $T$, which must be transferred to the adjoint app.  This derivative is used in the [Convective Boundary Condition Example](materialInv_ConvectiveBC.md).

### Thermal Conductivity id=sec:material_inversion

For a parameterized conductivity $\kappa(x;\mathbf{p})$, the stiffness term of [eq:discretePDE] depends on $\mathbf{p}$.  Differentiating it with respect to $p_j$ at fixed $T$ gives
\begin{equation}\label{eq:kappa}
\frac{\partial\mathcal{R}_\alpha}{\partial p_j} = \int_\Omega\nabla\phi_\alpha\cdot\frac{\partial \kappa}{\partial p_j}\nabla T~\text{d}\Omega ,
\end{equation}
and contracting with the adjoint replaces $\nabla\phi_\alpha$ with $\nabla\lambda$:
\begin{equation}\label{eq:kappaLambda}
\lambda^\top\frac{\partial\mathcal{R}}{\partial p_j} = \sum_\alpha\lambda_\alpha\int_\Omega\nabla\phi_\alpha\cdot\frac{\partial \kappa}{\partial p_j}\nabla T~\text{d}\Omega
= \int_\Omega\nabla\lambda\cdot\frac{\partial \kappa}{\partial p_j}\nabla T~\text{d}\Omega .
\end{equation}
The derivative given in [eq:kappaLambda] is used in this [example](materialInv_ConstK.md) where the thermal conductivity of a block is found.

Substituting [eq:kappaLambda] into [eq:adjointGrad] and neglecting regularization gives each gradient component as $\mathrm{d}f/\mathrm{d}p_j=\int_\Omega s_\kappa\,\partial\kappa/\partial p_j~\text{d}\Omega$, where $s_\kappa=-\nabla\lambda\cdot\nabla T$ is the +sensitivity density+, the change in $f$ per unit change in $\kappa$ at each point.  The density depends only on the forward and adjoint solutions; the parameterization of $\kappa$ enters only through the weight $\partial\kappa/\partial p_j$.  A single conductivity for the whole domain integrates $s_\kappa$ over $\Omega$, one conductivity per subdomain integrates it over each subdomain, and a conductivity interpolated from a parameter mesh, as in [ParameterMeshFunction.md], projects it onto the parameter mesh shape functions.  [ElementOptimizationDiffusionCoefFunctionInnerProduct.md] evaluates this integral with quadrature, using the $\partial\kappa/\partial p_j$ supplied by the optimization function.

The same procedure applies to a temperature dependent thermal conductivity,
\begin{equation}
\kappa\left(T\right)=\alpha T^2+\beta T,
\end{equation}
with design parameters $\alpha$ and $\beta$.  Since $\partial\kappa/\partial\alpha=T^2$ and $\partial\kappa/\partial\beta=T$, [eq:kappaLambda] gives
\begin{equation}\label{eq:TdependentKappa}
\lambda^\top\frac{\partial\mathcal{R}}{\partial\mathbf{p}} =
\left[\int_\Omega T^2\,\nabla\lambda\cdot\nabla T~\text{d}\Omega ,\ \int_\Omega T\,\nabla\lambda\cdot\nabla T~\text{d}\Omega\right] .
\end{equation}
When $\kappa$ depends on $T$, the Jacobian $\partial\mathcal{R}/\partial\mathbf{u}$ includes a $\mathrm{d}\kappa/\mathrm{d}T$ term and is no longer symmetric, so the adjoint problem [eq:adjoint] must use its transpose rather than the forward operator in [eqn:adjoint_problem].  [SteadyAndAdjoint.md] forms this transpose automatically.

!bibtex bibliography
