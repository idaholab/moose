# Inverse Optimization id=sec:invOpt

This page provides a practical overview of the theory and equations needed to solve inverse optimization problems for users/developers of the optimization module.  This overview focuses on optimization problems constrained by the solution to a partial differentiation equation, i.e. PDE constrained optimization, and how to efficiently compute the various gradients needed to accelerate the convergence of the optimization solver.  See citations throughout this overview for a more in depth and rigorous overview of inverse optimization.  This overview on inverse optimization in the optimization module is split up into the following sections:

- [PDE constrained Optimization Theory:](#sec:invOptTheory) Describes the theory for partial differential equation (PDE) constrained optimization
- [Adjoint Equation and Boundary Conditions:](#sec:adjoint) Derivation of the Adjoint equation using Lagrange multipliers and its boundary conditions.
- [Parameter Derivatives of PDE:](#sec:PDEDerivs) Derivatives of the PDE with respect to the optimization parameters for force and material inversion.  Force inversion examples where PDE constrained optimization is used to parameterize boundary conditions and body loads is provided in this [section](#sec:forceInvExample).  Material inversion examples are provided in this [section](#sec:forceInvExample) where PDE constrained optimization is used to control material properties.

The overall flow of the optimization algorithm implemented in the MOOSE optimization module is shown in [fig:optCycle].  In this example, the internal heat source, $q_v$, is being parameterized to match the simulated and experimental steady state temperature fields, $T$ and $\widetilde{T}$, respectively.  Step one of the optimization cycle consists of adjusting the internal heat source, $q_v$.  In step two, the physics model is solved with the current $q_v$ to obtain a simulated temperature field, $T$.  Step two is often referred to as the forward solve.  In step three, the simulated and experimental temperature fields are compared via the objective function, $f$.  If $f$ is below the user defined threshold, the optimization cycle stops and the best fit parameterization of $q_v$ is found.  If $f$ is above the user defined threshold, the optimization algorithm determines a new $q_v$ and the process is repeated.  In the next section, methods for determining the next iteration of the parameterized value, in this case $q_v$, will be presented.  Problems solved using the optimization module are available on the [Examples page](optimization/examples/index.md).

!media large_media/optimization/fig_optCycle.png
       style=width:80%;margin:auto;padding-top:2.5%;
       id=fig:optCycle
       caption=Optimization cycle example for parameterizaing an internal heat source distribution $q_v$ to match the simulated and experimental temperature field, $T$ and $\widetilde{T}$, respectively.

[comment0]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Constrained Inverse Optimization id=sec:invOptTheory

Inverse optimization is a mathematical framework to infer model parameters by minimizing the misfit between the experimental and simulation observables.  In the optimization module, our model is a PDE describing the physics of the experiment.  We solve our physics PDE using the finite element method as implemented in MOOSE.  The physics of our problem constrains our optimization algorithm.  A PDE-constrained inverse optimization framework is formulated as an abstract optimization problem [!citet](biegler2003large):
\begin{equation}\label{eq:optimization}
   \min_{\mathbf{p}} f\left(\mathbf{u},\mathbf{p}\right);\quad~\textrm{subject to}~\mathcal{R}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0} ,
\end{equation}
where $f(\mathbf{u},\mathbf{p})$ is our objective function which provides a scalar measure of the misfit between experimental and simulated responses, along with any regularization [!citet](neumaier1998solving).  The constraint, $\mathcal{R}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0}$, is the residual vector for the PDEs governing the multiphysics phenomena simulated by MOOSE (e.g. coupled heat and elasticity equations), the vector $\mathbf{p}$ contains design variables (e.g. material properties or loads) and the vector $\mathbf{u}$ contains state variables (e.g. temperature and displacement fields).  [eq:optimization] appears simple on the outset but is extremely difficult to solve. The solution space can span millions of degrees of freedom and the parameter space can also be very large. Finally, the PDEs can be highly nonlinear, time-dependent and tightly coupling complex phenomena across multiple physics.

Optimization problems can be solved using either global (gradient-free) or local (gradient-based) approaches [!citet](aster2018parameter). Global approaches require a large number of iterations compared to gradient-based approaches (e.g. conjugate gradient or Newton-type methods), making the latter more suitable to problems with a large parameter space and computationally expensive models.  The [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) optimization library [!citet](balay2019petsc) is used to solve [eq:optimization].  Optimization libraries like TAO require access to functions for computing the objective ($f$), gradient $\left(\mathrm{d}f/\mathrm{d}\mathbf{p}\right)$ and Hessian $\left(\mathrm{d}^2f/\mathrm{d}\mathbf{p}^2\right)$ or a function to take the action of the Hessian on a vector.  An objective function measuring the misfit or distance between the simulation and experimental data usually has the form
\begin{equation}\label{eq:objective}
f(\mathbf{u}, \mathbf{p}) = \frac{1}{2}\sum^N_i \left(u_i - \tilde u_i\right)^2 + \frac{\rho}{2}\sum^M_j p_jp_j ,
\end{equation}
where the first summation over $(i=1..N)$ is an L$_2$ norm or euclidean distance between experimental measurement points, $\mathbf{\tilde  u}$, and the simulated solution, $\mathbf{u}$.  The second summation over $(j=1..M)$ provides Tikhonov regularization of the design variables, $\mathbf{p}$, for ill-posed problems where $\rho$ controls the amount of regularization.  Other types of regularization may also be used.

Gradient-free optimization solvers only require a function to solve for the objective given in [eq:objective].  Solving for the objective only requires solving a forward problem to determine $\mathbf{u}$ and then plugging that into [eq:objective] to determine $f$.  The forward problem is defined as the FEM model of the experiment which the analyst should have already made before attempting to perform optimization.  The parameters that go into the forward problem (e.g. pressure distributions on sidesets or material properties) are adjusted by the optimization solver and the forward problem is recomputed.  This process continues until $f$ is below some user defined threshold. The basic gradient-free solver available in TAO is the simplex or Nelder-Mead method.  Gradient-free optimization solvers are robust and straight-forward to use.  Unfortunately, their computational cost scales exponentially with the number of parameters.  When the forward model is a computationally expensive FEM model, gradient-free approaches quickly become computationally expensive.

Gradient-based optimization algorithms require fewer iterations but require functions to solve for the gradient vector and sometimes Hessians matrix.  TAO has `petsc_options` to evaluate finite difference based gradients and Hessians by solving the objective function multiple times with perturbed parameters, which also requires multiple solves of the forward problem.  Finite difference gradients and Hessians are good for testing an optimization approach but become too expensive for realistic problems.

Given the large parameter space, we resort to the adjoint method for gradient computation; unlike finite difference approaches, the computational cost of adjoint methods is independent of the number of parameters [!citet](plessix2006review).  In the adjoint method, the gradient, i.e. the total derivative $\mathrm{d}f/\mathrm{d}\mathbf{p}$, is computed as,
\begin{equation}\label{eq:adjointGrad}
\frac{\mathrm{d}f}{\mathrm{d}\mathbf{p}} = \frac{\partial f}{\partial\mathbf{p}}+\mathbf{\lambda}^\top\frac{\partial\mathcal{R}}{\partial\mathbf{p}} ,
\end{equation}
where $\partial f/\partial\mathbf{p}$ accounts for the regularization in [eq:objective] and $\mathbf{\lambda}$ is the adjoint variable solved for from the adjoint equation
\begin{equation}\label{eq:adjoint}
\left(\frac{\partial\mathcal{R}}{\partial\mathbf{u}}\right)^\top \mathbf{\lambda}= -\left(\frac{\partial f}{\partial\mathbf{u}}\right)^\top ,
\end{equation}
where $\left(\partial\mathcal{R}/\partial\mathbf{u}\right)^\top$ is the adjoint of the Jacobian from the residual vector of the forward problem, $\mathcal{R}$, and $\left(\partial f/\partial\mathbf{u}\right)^\top$ is a body force like term that accounts for the misfit between the computed and experimental data.  Thus, the solution to [eq:adjoint] has the same complexity as the solution to the forward problem.

The remaining step for evaluating the derivative of the PDE in [eq:adjointGrad] is to compute $\partial\mathcal{R}/\partial\mathbf{p}$, the derivative of the PDE with respect to the parameter vector.  The form this term takes is dependent on the physics (e.g. mechanics or heat conduction) and the parameter being optimized (e.g. force inversion versus material inversion).  In what follows, we will derive the adjoint equation for steady state heat conduction and the gradient term for both force and material inversion.

In the following, the adjoint method given in [eq:adjointGrad] and [eq:adjoint] is derived from [eq:objective] following the derivation given by [!citet](bradley2013pde).  Neglecting the regularization in [eq:objective] the total derivative of $f$ with respect to $\mathbf{p}$ using the chain rule is given by
\begin{equation}\label{eq:objectiveGrad}
\frac{\mathrm{d}f(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} = \frac{\partial f}{\partial \mathbf{u}}\frac{\partial \mathbf{u}}{\partial\mathbf{p}}.
\end{equation}
We can solve easily compute  $\frac{\partial \mathbf{u}}{\partial\mathbf{p}}$ and so we need to rearrange  The physics constraint of the PDE from [eq:optimization], $\mathcal{R}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0}$, implies that
\begin{equation} \label{eq:gradZero}
	\begin{aligned}
	\frac{\mathrm{d}\mathcal{R}(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} & = 0  \\
	\frac{\mathrm{d}\mathcal{R}(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} & = \frac{\partial \mathcal{R}}{\partial \mathbf{u}}\frac{\partial \mathbf{u}}{\partial\mathbf{p}} + \frac{\partial \mathcal{R}}{\partial \mathbf{p}}= 0 \\
\end{aligned}
\end{equation}
Rearranging the last part of [eq:gradZero] gives
\begin{equation}\label{eq:gradZeroArrange}
\frac{\partial \mathbf{u}}{\partial\mathbf{p}}=-\left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)^{-1}\frac{\partial \mathcal{R}}{\partial \mathbf{p}}
\end{equation}
which can be substituted into [eq:objectiveGrad] to give
\begin{equation}\label{eq:objectiveGrad2}
\frac{\mathrm{d}f(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} = -\frac{\partial f}{\partial \mathbf{u}}\left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)^{-1}\frac{\partial \mathcal{R}}{\partial \mathbf{p}}.
\end{equation}
The terms in [eq:objectiveGrad2] are all terms we can compute.  The derivative of the objective with respect to the degree of freedom, $\frac{\partial f}{\partial \mathbf{u}}$, is dependent on the data being fit and will be discussed in the section discussing the [adjoint method](#sec:adjoint) for discrete data points.  The term, $\frac{\partial \mathcal{R}}{\partial \mathbf{p}}$, requires differentiation of our PDE with respect to each parameter being inverted for and is derived for simple cases in the following [section](#sec:PDEDerivs) for force inversion and material identification.  The middle term, $\frac{\partial \mathcal{R}}{\partial \mathbf{u}}$, is the Jacobian matrix of the forward problem.  [eq:objectiveGrad2] requires the inverse of the Jacobian matrix which is not how we actually solve this system of equations.

We have two options for dealing with the inverse of the Jacobian.  In the first, we could perform one linear solve per parameter, $p_i$, being fit by solving $\left[\left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)^{-1}\frac{\partial \mathcal{R}}{\partial p_i}\right]$.  This algorithm scales with the number of parameters which makes it computationally expensive for tens of parameters.

The alternative that we will use is the adjoint method which scales independently to the number of optimization parameters.   The adjoint method requires one additional solve of the same complexity as the original forward problem.  The adjoint equation given in [eq:adjoint] is found by setting the adjoint variable, $\lambda$, equal to the first two terms of [eq:objectiveGrad2] and then rearranging terms to give
\begin{equation} \label{eq:AdjEqn}
	\begin{aligned}
	&\lambda^\top= \frac{\partial f}{\partial \mathbf{u}}\left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)^{-1}  \\
	&\rightarrow \lambda^\top \left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)= -\frac{\partial f}{\partial \mathbf{u}}\left[\left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)^{-1} \left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)\right] \\
	&\rightarrow \lambda^\top \left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)= -\frac{\partial f}{\partial \mathbf{u}} \\
	&\rightarrow \left[\lambda^\top \left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)\right]^\top= -\left(\frac{\partial f}{\partial \mathbf{u}}\right)^\top \\
	&\rightarrow \left(\frac{\partial \mathcal{R}}{\partial \mathbf{u}}\right)^\top \lambda = -\left(\frac{\partial f}{\partial \mathbf{u}}\right)^\top \\
\end{aligned}
\end{equation}
where each step of the derivation has been included as a reminder of how [eq:adjoint] is obtained.  The next [section](#sec:adjoint) uses an alternative approach to determine the adjoint equation based on Lagrange multipliers [!citet](walsh2013source).

[comment1]: <> (% ----------------------------------------------------------------------------------------------------------%)

## Adjoint Problem for Steady State Heat Conduction id=sec:adjoint

In this section, we are interested in solving the following PDE-constrained optimization problem from [eq:optimization] for steady state heat conduction:
\begin{equation} \label{eqn:optimization_problem}
	\begin{aligned}
	& \min_{\mathbf{p}}
	& & f\left(T,\mathbf{p}\right) = \frac{1}{2} \sum_{i=1}^{N} \left( T_i - \widetilde {T}_i \right)^2 , \\
	& \text{subject to}
	& & \mathcal{R}\left(T,\mathbf{p}\right)=\nabla \cdot \kappa \nabla T + g_b =0, & \text{in}~\Omega , \\
\end{aligned}
\end{equation}
where $f$ is the objective function from [eq:objective] without regularization, $g_b$ is the distributed heat flux, $\widetilde{T}$ is the experimental temperature being compared to our simulation temperature field at discrete locations, $T_i$.  Other forms for the objective function are possible such as different norms or different types of measurements that may require integration over a line or volume.

We also have the following boundary conditions for our PDE,
\begin{equation} \label{eqn:optimization_bcs}
	\begin{aligned}
	& & & T = T_D, &\text{on}~\Gamma_D, \\
	& & & \left( \kappa \nabla T \right) \cdot \boldsymbol{n} = G\left(T\right), &\text{on}~\Gamma_R , \\
\end{aligned}
\end{equation}
where $\boldsymbol{n}$ is the normal vector, $\Gamma_D$ is the Dirichlet boundary, and $\Gamma_R$ is the Robin or mixed boundary. Common  cases for $G\left(T\right)$ are:
\begin{equation}
\begin{aligned}\label{eq:robin_bc_types}
   &\text{Neumann:     }~G\left(T\right) = g_n , \\
   &\text{Convection:  }~G\left(T\right) = h (T - T_{\infty}) , \\
\end{aligned}
\end{equation}
where $h$ is the heat transfer coefficient and $g_n$ is independent of $T$.
The objective function can also be expressed in a integral form for the case of $N$ point measurements as follows
\begin{equation}  \label{eqn:objective_integral}
	f = \frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \widetilde{T}_i \right)^2\text{d}\Omega.
\end{equation}

We take the equivalent, variational approach to derive the adjoint. Thus, the Lagrangian of this problem is
\begin{equation} \label{eqn:lagrangian}
\begin{aligned}
	\mathcal{L}(T, \mathbf{p}, \lambda) & = f + \int \left(  \nabla \cdot \kappa \nabla T + g_b  \right) \lambda~\text{d}\Omega \\
					        		& = f +  \int  \left(  g_b \lambda \right) ~\text{d}\Omega
								+\int  \left(  \nabla \cdot \kappa \nabla T \right) \lambda~\text{d}\Omega.
\end{aligned}
\end{equation}
The divergence theorem is applied to the last term in [eqn:lagrangian] giving
\begin{equation}
\begin{aligned}
	 \int  \left(  \nabla \cdot \kappa \nabla T \right) \lambda~\text{d}\Omega
	 & = \int \lambda \left( \kappa \nabla T  \right)\cdot \boldsymbol{n}~\text{d}\Gamma
	 	-\int \left(  \kappa\nabla \lambda \right)\cdot \nabla T~\text{d}\Omega\\
	 & = \int\left[ \lambda \left( \kappa \nabla T \right)\cdot \boldsymbol{n}
	 	- T\left( \kappa \nabla \lambda  \right) \cdot \boldsymbol{n}\right]~\text{d}\Gamma
	 	+ \int \left(\nabla\cdot \kappa \nabla \lambda \right)T ~\text{d}\Omega.
\end{aligned}
\end{equation}
By substituting the above and [eqn:objective_integral] into [eqn:lagrangian], we have
\begin{equation} \label{eqn:lagrangian_fn}
\begin{aligned}
	\mathcal{L}(T, \mathbf{p}, \lambda) =& \mathcal{A}(T, \mathbf{p}, \lambda) +\mathcal{B}(T, \mathbf{p}, \lambda), \\
	 \mathcal{A}(T, \mathbf{p}, \lambda)=&\frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \widetilde{T} \right)^2\text{d}\Omega
	 	+ \int \left(  g_b \lambda \right) ~\text{d}\Omega
	 	+ \int \left(\nabla\cdot \kappa \nabla \lambda\right)T ~\text{d}\Omega,\\
	 \mathcal{B}(T, \mathbf{p}, \lambda)=&  \int\left[ \lambda \left( \kappa \nabla T \right)\cdot \boldsymbol{n}
	   	- T\left( \kappa \nabla \lambda  \right) \cdot \boldsymbol{n}\right]~\text{d}\Gamma,
\end{aligned}
\end{equation}
where $\lambda$ is the Lagrange multiplier field known as the adjoint state or costate variable.  The Lagrangian has been broken up into terms integrated over the body, $\mathcal{A}$, and boundary terms, $\mathcal{B}$.  In order to determine the boundary conditions for the adjoint equation, the boundary integral terms, $\mathcal{B}$, in [eqn:lagrangian_fn] are further broken up into their separate domains, $\Gamma_D$ and $\Gamma_R$, given in [eqn:optimization_bcs] resulting in
\begin{equation}\label{eqn:bcLagrangian}
\begin{aligned}
\mathcal{B}=&\int\left[ \lambda \left( \kappa \nabla T \right)\cdot \boldsymbol{n}
                     	- T\left( \kappa \nabla \lambda  \right) \cdot \boldsymbol{n}\right]~\text{d}\Gamma\\
                  =&\int_{\Gamma_R}\lambda G(T)~\text{d}\Gamma
                    	+\int_{\Gamma_D}\lambda\kappa\nabla T\cdot\boldsymbol{n}~\text{d}\Gamma
    			-\int _{\Gamma_R}T\kappa\nabla\lambda\cdot\boldsymbol{n}~\text{d}\Gamma
    			-\int_{\Gamma_D} T_{o}\kappa\nabla\lambda\cdot\boldsymbol{n}~\text{d}\Gamma,\\
\end{aligned}
\end{equation}
where $T_o$ is the prescribed temperature on the Dirichlet boundary, $\Gamma_D$.  The variation of $\mathcal{L}$ with respect to $T$ is then given by $\delta\mathcal{L}=\delta\mathcal{A}+\delta\mathcal{B}$ where the variation of the body terms with respect to $T$ are given by
\begin{equation}\label{eqn:delta_bodyLagrangian}
\begin{aligned}
\delta\mathcal{A}=& \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \widetilde{T} \right)\delta T\text{d}\Omega + \int \left(\nabla\cdot \kappa \nabla \lambda \right) \delta T ~\text{d}\Omega\\
				=&\int \left(\left(\nabla\cdot \kappa \nabla \lambda \right) +  \sum_{i=1}^{N} \delta(x - x_i) \left( T - \widetilde{T} \right) \right)\delta T ~\text{d}\Omega,
\end{aligned}
\end{equation}
and the variation of $\mathcal{B}$ with respect to $T$ is given as
\begin{equation}\label{eqn:delta_bcLagrangian}
\begin{aligned}
\delta\mathcal{B}=&\int_{\Gamma_R}\lambda\frac{\text{d}G(T)}{\text{d}T}\delta T \text{d}\Gamma
                    		-\int_{\Gamma_R}\left(\kappa\nabla\lambda\cdot\boldsymbol{n}\right)\delta T\text{d}\Gamma
                           	+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma\\
=&\int_{\Gamma_R}\left(\lambda h -\left(\kappa\nabla\lambda\cdot\boldsymbol{n}\right)\right)\delta T\text{d}\Gamma
    				+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma,
\end{aligned}
\end{equation}
where $\text{d}G(T)/\text{d}T=h$ for the convection BC given in [eq:robin_bc_types].
Combining [eqn:delta_bodyLagrangian] and [eqn:delta_bcLagrangian] to get $\delta\mathcal{L}$ results in
\begin{equation}\label{eqn:delta_Lagrangian}
\begin{aligned}
\delta\mathcal{L}=&\delta\mathcal{A}+\delta\mathcal{B}\\
=&\int \left(\left(\nabla\cdot \kappa \nabla \lambda \right)
	+  \sum_{i=1}^{N} \delta(x - x_i) \left( T - \widetilde{T} \right) \right)\delta T ~\text{d}\Omega\\
    &+\int_{\Gamma_R}\left(\lambda h -\left(\kappa\nabla\lambda\cdot\boldsymbol{n}\right)\right)\delta T\text{d}\Gamma
    	+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma.
\end{aligned}
\end{equation}

Stationarity of $\mathcal{L}$ would require $\delta\mathcal{L} = 0$ for all admissible $\delta T$. Setting each of the integrals in [eqn:delta_Lagrangian] results in the adjoint problem and its boundary conditions
\begin{equation} \label{eqn:adjoint_problem}
\boxed{
\begin{aligned}
\nabla\cdot \kappa \nabla \lambda +  \sum_{i=1}^{N} \delta (x - x_i)(T - \widetilde{T}) &=0, ~\text{in}~\Omega,\\
\lambda  &= 0, ~\text{on}~\Gamma_D, \\
\kappa \nabla \lambda \cdot \boldsymbol{n} & = \lambda h, ~\text{on}~{\Gamma_R}. \\
\end{aligned}
}
\end{equation}
Solving [eqn:adjoint_problem] comes down to adjusting the boundary conditions and load vector from the forward problem and re-solving.

[comment2]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivatives for Inversion id=sec:PDEDerivs

In this section we will present derivatives for steady state heat conduction [eqn:optimization_problem] with respect to the force or material parameters.  For all of these examples, measurement data is taken at specific locations where the objective function can be represented by [eqn:objective_integral].  We will present the discrete forms of our PDE and its derivative which most closely matches the implementation that will be used in MOOSE.

The discrete form of the PDE constraint for steady state heat conduction in [eqn:optimization_problem] is given by the the residual, $\hat{\text R}$, as

\begin{equation}\label{eq:discretePDE}
\hat{\text R}=\textbf{J}\hat{\text T} - \hat{\text g}=0,
\end{equation}
where $\textbf{J}$ is the Jacobian matrix, $\hat{\text T}$ and $\hat{\text g}$ are the discretized temperature and source vectors. Element-wise definitions of the terms are
\begin{equation} \label{eq:elementwise_discretized_terms}
\begin{aligned}
    \textbf{J}^{\alpha\beta}&=\sum_{e}\int\nabla^\top \phi^{\alpha} \cdot \kappa\cdot\nabla\phi^{\beta}~\text{d}\Omega,\\
    \hat{\text f}^{\alpha} & = \sum_{e} \int \phi^{\alpha} g_b \text{d}\Omega + \sum_{e} \int \phi^{\alpha}G(T) ~\text{d}\Gamma_R,
\end{aligned}
\end{equation}
where $\phi^{\alpha}$ denotes the continuous Lagrange finite element shape function at node $\alpha$. The solution can be expressed as $T(x)\approx \sum_{\alpha} \phi^{\alpha}\hat{\text T}^{\alpha} = \phi \hat{\text T}$. Note here the $\hat{\text g}$ includes the contribution from both the body load term ($g_b$ in [eqn:optimization_problem]) and flux boundary conditions (see [eqn:optimization_bcs]). We are assuming a Galerkin form for our discretized PDE by making our test and trial functions identical (both are $\phi^{\alpha}$). Note in the rest of this document, we omit the superscript of the shape function (i.e., $(\cdot)^{\alpha}$) and the summation over all the elements (i.e., $\sum_e$) for simplicity.

To compute the derivative of PDE with respect to the design parameters, i.e., $\partial \mathcal{R} /\partial \boldsymbol{p}$, it can be seen from [eq:elementwise_discretized_terms] that the derivatives can come from either $\textbf{J}$ or $\hat{\text f}$. For problems that have design parameters that are embedded in $g_b$ and/or the Neumann boundary condition in $G(T)$, we call them +force inversion+ problems, since the derivative only depends on the body load. For design parameters that are embedded in $\textbf{J}$ and/or the convection boundary condition in $G(T)$, indicating dependence on the material property, we call them +material inversion+ problems. Derivative calculation of different force inversion and material inversion problems are included in the following subsections:

- [#sec:forceInv]
- [#sec:robinInv]
- [#sec:material_inversion]

Examples for force and material inversion are given on the following pages:


[comment3]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivatives for Force Inversion id=sec:forceInv

We will first consider the simple case of load parameterization for body loads, $g_b$ (in [eqn:optimization_problem]), where the gradient is given as
\begin{equation}\label{eq:bodyLoads}
\begin{aligned}
\frac{\partial\hat{\text{R}}}{\partial \boldsymbol{p}} & =  \frac{\partial\hat{\text{R}}}{\partial \hat{g}} \frac{\partial\hat{g}}{\partial g_b} \frac{\partial g_b}{\partial \boldsymbol{p}}\\
& = \frac{\partial\hat{g}}{\partial g_b} \frac{\partial g_b}{\partial \boldsymbol{p}} \\
& = \int \phi\cdot\frac{\partial g_b(x)}{\partial\mathbf{p}}~\text{d}\Omega,\\
\end{aligned}
\end{equation}
by taking the chain rule from [eq:discretePDE] and [eq:elementwise_discretized_terms].
The gradient term requires the derivative of $g_b$ to be integrated over the volume $\Omega$.

For the first case, we will consider a volumetric heat source that varies linearly across the body, which is given by
\begin{equation}
g_b(x)=p_0+p_1x,
\end{equation}
with $p_0$ and $p_1$ the design parameters. Therefore, the derivative can be calculated as
\begin{equation}\label{eq:bodyLoad_example}
\begin{aligned}
\frac{\partial \hat{\text{R}}}{\partial\mathbf{p}} &= \int \phi\cdot\frac{\partial g_b(x)}{\partial\mathbf{p}}~\text{d}\Omega\\
&=
	\left[\int \phi\cdot \frac{\partial g_b}{\partial{p_0}}~\text{d}\Omega
	, \int \phi\cdot \frac{\partial g_b}{\partial{p_1}}~\text{d}\Omega\right]^\top\\
&= \left[\int\phi\cdot \left(1\right)~\text{d}\Omega
    ,\int\phi\cdot \left(x\right)~\text{d}\Omega\right]^\top.\\
\end{aligned}
\end{equation}
An example using the derivative given in [eq:bodyLoad_example] is given in the [Distributed Body Load Example ](forceInv_BodyLoad.md).

In the next force inversion case, we parameterize the intensity of $N$ point heat sources, where the heat source takes the form
\begin{equation}
g_b(x)=\sum_{i=1}^{np}\delta\left(x-x_i \right)p_i  \quad \text{for}~i=1 \dots N.
\end{equation}
The corresponding gradient term is given by
\begin{equation}\label{eq:pointLoad}
\begin{aligned}
 \\
\frac{\partial \hat{\text{R}}}{\partial p_i} &= \int \phi\cdot\frac{\partial g_b(x)}{\partial{p}_i}~\text{d}\Omega\\
& = \int \phi\cdot\delta\left(x-x_i \right)d\Omega \quad \text{for}~i=1 \dots N,
\end{aligned}
\end{equation}
which makes the gradient equal to one at the locations of the point loads.  This PDE derivative is used in the [Point Loads Example](forceInv_pointLoads.md).

Next, we will use force inversion to parameterize a Neumann boundary condition with the heat flux on the boundary being a function of the coordinates, $G(x)$, given by
\begin{equation}\label{eq:bcLoad}
\begin{aligned}
\frac{\partial\hat{\text{R}}}{\partial \boldsymbol{p}} & =  \frac{\partial\hat{\text{R}}}{\partial \hat{g}} \frac{\partial\hat{g}}{\partial G(x)} \frac{\partial G(x)}{\partial \boldsymbol{p}}\\
& = \frac{\partial\hat{g}}{\partial G(x)} \frac{\partial G(x)}{\partial \boldsymbol{p}} \\
& = \int_{\Gamma_R} \phi\cdot\frac{\partial G(x)}{\partial\mathbf{p}}~\text{d}\Gamma,\\
\end{aligned}
\end{equation}
where the derivative of $G(x)$ is now integrated over the boundary $\Gamma_R$. For instance, if we have a linearly varying heat flux
\begin{equation}
G(x)=p_0 + p_1x,
\end{equation}
with $p_0$ and $p_1$ the design parameters, then
\begin{equation}\label{eq:neumannBC}
\begin{aligned}
\frac{\partial\hat{\text{R}}}{\partial \boldsymbol{p}} &= \left[ \frac{\partial\hat{\text{R}}}{\partial {p}_0}, \frac{\partial\hat{\text{R}}}{\partial {p}_1} \right]^\top  \\
 &=   \left[\int_{\Gamma_R} \phi\cdot \frac{\partial G(x)}{\partial p_0} ~\text{d}\Gamma
    ,\int_{\Gamma_R} \phi\cdot \frac{\partial G(x)}{\partial p_1} ~\text{d}\Gamma\right]^\top \\
&= \left[\int_{\Gamma_R} \phi~\text{d}\Gamma
    ,\int_{\Gamma_R} \phi x~\text{d}\Gamma\right]^\top.\\
\end{aligned}
\end{equation}
The derivative given in [eq:neumannBC] is used in the [Neumann Boundary Condition Example](forceInv_NeumannBC.md).

The above force inversion examples, [eq:bodyLoads] and [eq:neumannBC], are all linear optimization problems where the parameter being optimized does not show up in the derivative term.  Linear optimization problems are not overly sensitive to the location of measurement points or the initial guesses for the parameter being optimized, making them easy to solve.  In the following we parameterize a Gaussian body force given by
\begin{equation}\label{eq:gaussOpt}
\begin{aligned}
g_b(x) = a \cdot \exp{\left( -\frac{(x-b)^2}{2c^2} \right)},
\end{aligned}
\end{equation}
where $a$ is the height or intensity of the Gaussian curve, $b$ is the location of the peak of the curve, and $c$ is the standard deviation of the curve or its width.  Parameterizing a Gaussian curve can result in a linear or nonlinear optimization problem depending on which parameter is being optimized.  Parameterizing this function for the height, $a$, results in the following linear optimization problem derivative
\begin{equation}\label{eq:gaussOpt_a}
\begin{aligned}
\frac{\partial\hat{\text{R}}}{\partial \boldsymbol{p}}
& = \frac{\partial\hat{\text{R}}}{\partial a}  = \int \phi\cdot\frac{\partial g_b(x)}{\partial a}~\text{d}\Omega\\
 &= \int \phi\cdot  \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}~\text{d}\Omega,
\end{aligned}
\end{equation}
where the parameter $a$ being optimized does not show up in the derivative term.
However, if we try to parameterize for the location of the peak of the Gaussian curve, $b$, we get the following nonlinear optimization problem derivative
\begin{equation}\label{eq:gaussOpt_b}
\begin{aligned}
\frac{\partial\hat{\text{R}}}{\partial \boldsymbol{p}} & = \frac{\partial\hat{\text{R}}}{\partial b} = \int \phi\cdot\frac{\partial g_b(x)}{\partial b}~\text{d}\Omega\\
&=\int \phi\cdot \frac{a(x-b)}{c^2} \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}~\text{d}\Omega,
\end{aligned}
\end{equation}
where the parameter $b$ remains in the derivative term.  Optimizing for the peak location of a Gaussian heat source, [eq:gaussOpt_b],  will be a much more difficult problem to solve than [eq:gaussOpt_a] and convergence will be dependent on the initial guesses given for $b$ and the locations where measurement data is taken.

[comment4]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivative for Convective Boundary Conditions id=sec:robinInv

In this section, the convective heat transfer coefficient, $h$, is considered as a parameter for the convection boundary condition given in [eq:robin_bc_types].  This is a material inversion problem with integral limited to the boundary, $\Gamma_R$.  The boundary condition is given by $G(T)=h \left(T - T_{\infty}\right)$ on $\Gamma_R$.  The PDE derivative term is given by
\begin{equation}\label{eq:convectiveBC}
\begin{aligned}
\frac{\partial\hat{\text{R}}}{\partial \boldsymbol{p}} &= \frac{\partial\hat{\text{R}}}{\partial h}
= \int_{\Gamma_R} \phi\cdot\frac{\partial G(x)}{\partial h}~\text{d}\Gamma\\
 &= \int_{\Gamma_R} \phi\cdot \left(T-T_{\infty} \right)~\text{d}\Gamma.
\end{aligned}
\end{equation}
This derivative requires the solution from the forward problem $T$ to be included in the integral over the boundary $\Gamma_R$ making this a nonlinear optimization problem.  This derivative is used in the [Convective Boundary Condition Example](materialInv_ConvectiveBC.md).

[comment5]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivative for Material Inversion id=sec:material_inversion

In [force inversion](#sec:forceInv) and [convective heat transfer inversion](#sec:robinInv), the design parameters only exist in the load terms. For force inversion problems, the derivatives of the residual, $\mathcal{R}$, with respect to the design parameters only required differentiation of the load functions, not the Jacobian.  In this section, we consider material inversion where the design parameters exist in the Jacobian term requiring it to be differentiated.  In the below derivatives, we treat the thermal conductivity ($\kappa$) as the design parameter by matching experimentally measured temperature data points. The derivative of $\mathcal{R}$ is taken with respect to $\kappa$. This requires the derivative of $\textbf{J}$ in [eq:discretePDE] leading to
\begin{equation}\label{eq:kappa}
\begin{aligned}
\frac{\partial \hat{\text{R}}}{\partial\mathbf{p}} &=  \int\nabla\phi^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla\phi~\text{d}\Omega \cdot\hat{\text T}\\
    &=  \int\nabla\phi^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla T~\text{d}\Omega,
\end{aligned}
\end{equation}
where $\nabla\phi\hat{T}=\nabla T$ was used in the last line.  The gradient term given by [eq:kappa] is multiplied by the adjoint variable $\lambda$ in [eq:adjointGrad], giving

\begin{equation}\label{eq:kappaLambda}
\begin{aligned}
\lambda^\top\frac{\partial \hat{\text{R}}}{\partial\mathbf{p}} &=  \hat{\lambda}^{\top}\cdot\int\nabla\phi^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla\phi~\text{d}\Omega \cdot\hat{\text T}\\
    &=  \int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla T~\text{d}\Omega,
\end{aligned}
\end{equation}
where  $\hat{\lambda}^{\top}\cdot\nabla\phi^{\top}=\left[\nabla\phi\cdot\hat{\lambda}\right]^{\top}=\left[\nabla\lambda\right]^{\top}=\nabla\lambda^{\top}$ is used in the last line.  Computing the integral in [eq:kappaLambda] requires an inner product of the gradients of the adjoint and forward variables. This is much simpler to compute in MOOSE than the integral in [eq:kappa] which requires the multiplication of nodal variables with elemental shape function gradients.  Material inversion is a nonlinear optimization problem since $T$ shows up in the derivative, making the derivative dependent on the solution to the forward problem.  The derivative given in [eq:kappaLambda] is used in this [example](materialInv_ConstK.md) where the thermal conductivity of a block is found.

This same procedure is used to a temperature dependent thermal conductivity given by
\begin{equation}
\kappa\left(T\right)=\alpha T^2+\beta T,
\end{equation}
where $\alpha$ and $\beta$ are the design parameters. The resultant derivative is then
\begin{equation}\label{eq:TdependentKappa}
\begin{aligned}
\lambda\frac{\partial \hat{\text{R}}}{\partial\mathbf{p}}=
    \lambda\left[ \frac{\partial \hat{\text{R}}}{\partial\alpha},\frac{\partial \hat{\text{R}}}{\partial\beta}\right]^\top
    &= \left[\int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{\partial\alpha}\cdot\nabla T~\text{d}\Omega
        ,\int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{\partial\beta}\cdot\nabla T~\text{d}\Omega\right]^\top\\
    &= \left[\int\nabla\lambda^{\top} \cdot \left(T^2\right) \cdot\nabla T~\text{d}\Omega,\int\nabla\lambda^{\top} \cdot \left(T\right) \cdot\nabla T~\text{d}\Omega\right]^\top.
\end{aligned}
\end{equation}

!bibtex bibliography
