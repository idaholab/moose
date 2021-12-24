# Inverse Optimization id=sec:invOpt

This section gives an overview of the inverse optimization capabilities developed in isopod. Section [#sec:invOptTheory] describes the theory for partial differential equation (PDE) constrained constrained optimization.  Gradient terms needed for gradient based optimization are derived in Section [#sec:adjoint] for the adjoint equation and PDE derivatives are given in Section [#sec:PDEDerivs].  Force and material inversion examples are provided on this [page](#sec:forceInvExample) where PDE constrained optimization is used to parameterize boundary conditions and body loads is provided on this [page](#sec:forceInvExample).  Material inversion examples are provided on this [page](#sec:forceInvExample) where PDE constrained optimization is used to identify material properties.

The over all flow of the optimization algorithm implemented in the MOOSE based app isopod is shown in [fig:optCycle].  In this example, the internal heat source, $q_v$, is being parameterized to match the simulated and experimental steady state temperature fields, $\widetilde{T}$ and $T$, respectively.  Step one of the optimization cycle consists of adjusting the internal heat source, $q_v$.  In step two, the physics model is solved with the current $q_v$ to obtain a simulated temperature field, $T$.  In step three, the simulated and experimental temperature fields are compared via the objective function, $J$.  If $J$ is below the user defined threshold, the optimization cycle stops and the best fit parameterization of $q_v$ is found.  If $J$ is above the user defined threshold, the optimization algorithm determines a new $q_v$ and the process is repeated.  In the next section, methods for determining the next iteration of the parameterized value, in this case $q_v$, will be presented.  

!media media/fig_optCycle.png
       style=width:80%;margin:auto;padding-top:2.5%;
       id=fig:optCycle
       caption=Optimization cycle example for parameterizaing an internal heat source distribution $q_v$ to match the simulated and experimental temperature field, $T$ and $\widetilde{T}$, respectively.

[comment]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Constrained Inverse Optimization id=sec:invOptTheory

Inverse optimization is a mathematical framework to infer model parameters by minimizing the misfit between the experimental and simulation observables.  In this work, our model is a PDE describing the physics of the experiment.  We solve our physics PDE using the finite element method as implemented in MOOSE.  The physics of our problem constrains our optimization algorithm.  A PDE-constrained inverse optimization framework is formulated as an abstract optimization problem [!citet](biegler2003large):
\begin{equation}\label{eq:optimization}
   \min_{\mathbf{p}} J\left(\mathbf{u},\mathbf{p}\right);\quad~\textrm{subject to}~\mathbf{g}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0} ,
\end{equation}
where $J(\mathbf{u},\mathbf{p})$ is our objective function, which is a scalar measure of the misfit between experimental and simulated responses, along with any regularization [!citet](neumaier1998solving).  The constraint, $\mathbf{g}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0}$, consists of the PDEs governing the multiphysics phenomena simulated by MOOSE (e.g. coupled heat and elasticity equations), $\mathbf{p}$ contains model parameters (e.g. material properties or loads) and $\mathbf{u}$ contains simulated responses (e.g. temperature and displacement fields).  [eq:optimization] appears simple on the outset but is extremely difficult to solve. The solution space can span millions of degrees of freedom and the parameter space can also be very large. Finally, the PDEs can be highly nonlinear, time-dependent and tightly coupling complex phenomena across multiple physics.

Optimization problems can be solved using either global (gradient-free) or local (gradient-based) approaches [!citet](aster2018parameter). Global approaches require a large number of iterations compared to gradient-based approaches (e.g. conjugate gradient or Newton-type methods), making the latter more suitable to problems with a large parameter space and computationally expensive models.  The [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) optimization library [!citet](balay2019petsc) is used to solve [eq:optimization].  Optimization libraries like TAO require access to functions for computing the objective ($J$), gradient $\left(\mathrm{d}J/\mathrm{d}\mathbf{p}\right)$ and Hessian $\left(\mathrm{d}^2J/\mathrm{d}\mathbf{p}^2\right)$ or a function to take the action of the Hessian on a vector.  An objective function measuring the misfit or distance between the simulation and experimental data usually has the form
\begin{equation}\label{eq:objective}
J(\mathbf{u}, \mathbf{p}) = \frac{1}{2}\sum_i \left(\mathbf{u}_i - \mathbf{\bar u}_i\right)^2 + \frac{\rho}{2}\sum_i \mathbf{p}^2 ,
\end{equation}
where the first integral is an L$_2$ norm or euclidean distance between the experimental data, $\mathbf{\bar u}$, and the simulated solution, $\mathbf{u}$.  The second integral provides Tikhonov regularization on the parameters, $\mathbf{p}$, for ill-posed problems where $\rho$ controls the amount of regularization.  Other types of regularization may also be used.

Gradient-free optimization solvers only require a function to solve for the objective given in [eq:objective].  Solving for the objective only requires solving a forward problem to determine $\mathbf{u}$ and then plugging that into [eq:objective] to determine $J$.  The forward problem is defined as the FEM model of the experiment which the analyst should have already made before attempting to perform optimization.  The parameters that go into the forward problem (e.g. pressure distributions on sidesets or material properties) are adjusted by the optimization solver and the forward problem is recomputed.  This process continues until $J$ is below some user defined threshold. The basic gradient-free solver available in TAO is the simplex or Nelder-Mead method.  Gradient-free optimization solvers are robust and straight-forward to use.  Unfortunately, their computational cost scales exponentially with the number of parameters.  When the forward model is a computationally expensive FEM model, gradient-free approaches quickly become computationally expensive.

Gradient-based optimization algorithms require fewer iterations but require functions to solve for the gradient vector and sometimes Hessians matrix.  TAO has \code{petsc\_options} to evaluate finite difference based gradients and Hessians by solving the objective function multiple times with perturbed parameters, which also requires multiple solves of the forward problem.  Finite difference gradients and Hessians are good for testing an optimization approach but become computationally expensive for realistic problems.

Neglecting the regularization in [eq:objective] the total derivative of $J$ with respect to $\mathbf{p}$ using the chain rule is given by
\begin{equation}\label{eq:objectiveGrad}
\frac{\mathrm{d}J(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} = \frac{\partial J}{\partial \mathbf{u}}\frac{\partial \mathbf{u}}{\partial\mathbf{p}}.  
\end{equation}
The physics constraint of the PDE from [eq:optimization], $\mathbf{g}\left(\mathbf{u},\mathbf{p}\right)=\mathbf{0}$, implies that
\begin{equation} \label{eq:gradZero}
	\begin{aligned}
	\frac{\mathrm{d}g(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} & = 0  \\
	\frac{\mathrm{d}g(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} & = \frac{\partial g}{\partial \mathbf{u}}\frac{\partial \mathbf{u}}{\partial\mathbf{p}} + \frac{\partial g}{\partial \mathbf{p}}= 0 \\
\end{aligned}
\end{equation}
Rearranging the last term in [eq:gradZero] gives
\begin{equation}\label{eq:gradZeroArrange}
\frac{\partial \mathbf{u}}{\partial\mathbf{p}}=-\left(\frac{\partial g}{\partial \mathbf{u}}\right)^{-1}\frac{\partial g}{\partial \mathbf{p}}
\end{equation}
which can be substituted into [eq:objectiveGrad] to give
\begin{equation}\label{eq:objectiveGrad2}
\frac{\mathrm{d}J(\mathbf{u},\mathbf{p})}{\mathrm{d}\mathbf{p}} = -\frac{\partial J}{\partial \mathbf{u}}\left(\frac{\partial g}{\partial \mathbf{u}}\right)^{-1}\frac{\partial g}{\partial \mathbf{p}}.  
\end{equation}



Given the large parameter space, we resort to the adjoint method for gradient computation; unlike finite difference approaches, the computational cost of adjoint methods is independent of the number of parameters (ref18).  In the adjoint method, the gradient, i.e. the total derivative $\mathrm{d}J/\mathrm{d}\mathbf{p}$, is computed as,
\begin{equation}\label{eq:adjointGrad}
\frac{\mathrm{d}J}{\mathrm{d}\mathbf{p}} = \frac{\partial J}{\partial\mathbf{p}}+\mathbf{\lambda}\frac{\partial\mathbf{g}}{\partial\mathbf{p}} ,
\end{equation}
where $\partial J/\partial\mathbf{p}$ accounts for the regularization in [eq:objective] and $\mathbf{\lambda}$ is the adjoint variable solved for from the adjoint equation
\begin{equation}\label{eq:adjoint}
\left(\frac{\partial\mathbf{g}}{\partial\mathbf{u}}\right)^\top \mathbf{\lambda}= \left(\frac{\partial J}{\partial\mathbf{u}}\right)^\top ,
\end{equation}
where $\left(\partial\mathbf{g}/\partial\mathbf{u}\right)^\top$ is the adjoint of the Jacobian for the original forward problem, $\mathbf{g}$, and $\left(\partial J/\partial\mathbf{u}\right)^\top$ is a body force like term that accounts for the misfit between the computed and experimental data.  Thus, the solution to \eq{eq:adjoint} has the same complexity as the solution to the forward problem.  

The remaining step for evaluating the derivative of the PDE in [eq:adjointGrad] is to compute $\partial\mathbf{g}/\partial\mathbf{p}$, the derivative of the PDE with respect to the parameter vector.  The form this term takes is dependent on the physics (e.g. mechanics or heat conduction) and the parameter being optimized (e.g. force inversion versus material inversion).  In what follows, we will derive the adjoint equation for steady state heat conduction and the gradient term for both force and material inversion.  

[comment]: <> (% ----------------------------------------------------------------------------------------------------------%)

## Adjoint Problem for Steady State Heat Conduction id=sec:adjoint

In this section, we are interested in solving the following PDE-constrained optimization problem from [eq:optimization] for steady state heat conduction:
\begin{equation} \label{eqn:optimization_problem}
	\begin{aligned}
	& \min_{\mathbf{p}}
	& & J\left(T,\mathbf{p}\right) = \frac{1}{2} \sum_{i=1}^{N} \left( T_i - \bar{T}_i \right)^2 , \\
	& \text{subject to}
	& & g\left(T,\mathbf{p}\right)=\nabla \cdot \kappa \nabla T + f_b =0, & \text{in}~\Omega , \\
\end{aligned}
\end{equation}
where $J$ is the objective function from [eq:objective] without regularization, $f_b$ is the distributed heat flux, $T$ is the experimental temperature field being compared to our simulation temperature at discrete locations, $\bar{T}_i$.  Other forms for the objective function are possible such as different norms or different types of measurements that may require integration over a line or volume.

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
   &\text{Neumann:     }~G\left(T\right) = G = f_n , \\
   &\text{Convection:  }~G\left(T\right) = h (T - T_{\infty}) , \\
\end{aligned}
\end{equation}
where $h$ is the heat transfer coefficient and $f_n$ is independent of $T$.
The objective function can also be expressed in a integral form for the case of $N$ point measurements as follows
\begin{equation}  \label{eqn:objective_integral}
	J = \frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \bar{T}_i \right)^2\text{d}\Omega.
\end{equation}

We take the equivalent, variational approach to derive the adjoint. Thus, the Lagrangian of this problem is
\begin{equation} \label{eqn:lagrangian}
\begin{aligned}
	\mathcal{L}(T, \mathbf{p}, \lambda) & = J + \int \left(  \nabla \cdot \kappa \nabla T + f_b  \right) \lambda~\text{d}\Omega \\
					        		& = J +  \int  \left(  f_b \lambda \right) ~\text{d}\Omega
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
	 \mathcal{A}(T, \mathbf{p}, \lambda)=&\frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \bar{T} \right)^2\text{d}\Omega 
	 	+ \int \left(  f_b \lambda \right) ~\text{d}\Omega  
	 	+ \int \left(\nabla\cdot \kappa \nabla \lambda\right)T ~\text{d}\Omega,\\
	 \mathcal{B}(T, \mathbf{p}, \lambda)=&  \int\left[ \lambda \left( \kappa \nabla T \right)\cdot \boldsymbol{n} 
	   	- T\left( \kappa \nabla \lambda  \right) \cdot \boldsymbol{n}\right]~\text{d}\Gamma,
\end{aligned}
\end{equation}
where $\lambda$ is the Lagrange multiplier field known as the adjoint state or costate variable.  The Lagrangian has been broken up into terms integrated over the body, $\mathcal{A}$, and boundary terms, $\mathcal{B}$.  In order to determine the boundary conditions for the adjoint equation, the boundary integral terms, $\mathcal{B}$, in \eq{eqn:lagrangian_fn} are further broken up into their separate domains, $\Gamma_D$ and $\Gamma_R$, given in [eqn:optimization_bcs] resulting in
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
\delta\mathcal{A}=& \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \bar{T} \right)\delta T\text{d}\Omega + \int \left(\nabla\cdot \kappa \nabla \lambda \right) \delta T ~\text{d}\Omega\\
				=&\int \left(\left(\nabla\cdot \kappa \nabla \lambda \right) +  \sum_{i=1}^{N} \delta(x - x_i) \left( T - \bar{T} \right) \right)\delta T ~\text{d}\Omega,
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
where $\text{d}G(T)/\text{d}T=h$ from [eqn:optimization_bcs] was used.
Combining [eqn:delta_bodyLagrangian] and [eqn:delta_bcLagrangian] to get $\delta\mathcal{L}$ results in 
\begin{equation}\label{eqn:delta_Lagrangian}
\begin{aligned}
\delta\mathcal{L}=&\delta\mathcal{A}+\delta\mathcal{B}\\
=&\int \left(\left(\nabla\cdot \kappa \nabla \lambda \right) 
	+  \sum_{i=1}^{N} \delta(x - x_i) \left( T - \bar{T} \right) \right)\delta T ~\text{d}\Omega\\
    &+\int_{\Gamma_R}\left(\lambda h -\left(\kappa\nabla\lambda\cdot\boldsymbol{n}\right)\right)\delta T\text{d}\Gamma
    	+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma.
\end{aligned}
\end{equation}

Stationarity of $\mathcal{L}$ would require $\delta\mathcal{L} = 0$ for all admissible $\delta T$. Setting each of the integrals in [eqn:delta_Lagrangian] results in the adjoint problem and its boundary conditions
\begin{equation} \label{eqn:adjoint_problem}
\boxed{
\begin{aligned}
\nabla\cdot \kappa \nabla \lambda +  \sum_{i=1}^{N} \delta (x - x_i)(T - \bar{T}) &=0, ~\text{in}~\Omega,\\
\lambda  &= 0, ~\text{on}~\Gamma_D, \\
\kappa \nabla \lambda \cdot \boldsymbol{n} & = \lambda h, ~\text{on}~{\Gamma_R}. \\
\end{aligned}
}
\end{equation}
Solving [eqn:adjoint_problem] comes down to adjusting the boundary conditions and load vector from the forward problem and re-solving.  

[comment]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivatives for Inversion id=sec:PDEDerivs

In this section we will present derivatives for steady state heat conduction [eqn:optimization_problem] with respect to the force or material parameters.  For all of these examples, measurement data is taken at specific locations where the objective function can be represented by [eqn:objective_integral].  We will present the discrete forms of our PDE and its derivative which most closely matches the implementation that will be used in MOOSE.  

The discrete form of the PDE constraint for steady state heat conduction in [eqn:optimization_problem], $\hat{g}$, is given as

\begin{equation}\label{eq:discretePDE}
\hat{\text g}=\textbf{K}\hat{\text T} - \hat{\text f}=0,
\end{equation}
where $\textbf{K}$ is the Jacobian matrix, $\hat{\text T}$ and $\hat{\text f}$ are the discretized  temperature and residual vectors. Element-wise definitions of the terms are
\begin{equation} \label{eq:elementwise_discretized_terms}
\begin{aligned}
    \textbf{K}^{\alpha\beta}&=\sum_{e}\int\nabla^\top \text{N}^{\alpha} \cdot \kappa\cdot\nabla\text{N}^{\beta}~\text{d}\Omega,\\
    \hat{\text f}^{\alpha} & = \sum_{e} \int N^{\alpha} f_b \text{d}\Omega + \sum_{e} \int N^{\alpha}G(T) ~\text{d}\Gamma_R,
\end{aligned}
\end{equation}
where $N^{\alpha}$ denotes the finite element shape function at node $\alpha$. The solution can be expressed as $T(x)\approx \sum_{\alpha} \text{N}^{\alpha}\hat{\text T}^{\alpha} = N \hat{\text T}$. Note here the $\hat{\text f}$ includes the contribution from both the body load term ($f_b$ in [eqn:optimization_problem]) and boundary conditions (see [eqn:optimization_bcs]). We are assuming a Galerkin form for our discretized PDE by making our test and trial functions identical (both are $\text{N}^{\alpha}$). Note in the rest of this document, we omit the superscript of the shape function (i.e., $(\cdot)^{\alpha}$) and the summation over all the elements (i.e., $\sum_e$) for simplicity.

To compute the derivative of PDE with respect to the design parameters, i.e., $\partial g /\partial \boldsymbol{p}$, it can be seen from [eq:elementwise_discretized_terms] that the derivatives can come from either $\textbf{K}$ or $\hat{\text f}$. For problems that have design parameters that are embedded in $f_b$ and/or the Neumann boundary condition in $G(T)$, we call them \textbf{force inversion} problems, since the derivative only depends on the body load. For design parameters that are embedded in $\textbf{K}$ and/or the convection boundary condition in $G(T)$, indicating dependence on the material property, we call them \textbf{material inversion} problems. Derivative calculation of different force inversion and material inversion problems are included in the following subsections ([#sec:forceInv], [#sec:robinInv], and [#sec:material_inversion]).

[comment]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivatives for Force Inversion id=sec:forceInv

We will first consider the simple case of load parameterization for body loads, $f_b$ (in [eqn:optimization_problem]), where the gradient is given as
\begin{equation}\label{eq:bodyLoads}
\begin{aligned}
\frac{\partial\hat{\text{g}}}{\partial \boldsymbol{p}} & =  \frac{\partial\hat{g}}{\partial \hat{f}} \frac{\partial\hat{f}}{\partial f_b} \frac{\partial f_b}{\partial \boldsymbol{p}}\\
& = \frac{\partial\hat{f}}{\partial f_b} \frac{\partial f_b}{\partial \boldsymbol{p}} \\
& = \int \text{N}\cdot\frac{\partial f_b(x)}{\partial\mathbf{p}}~\text{d}\Omega,\\
\end{aligned}
\end{equation}
by taking the chain rule from [eq:discretePDE] and [eq:elementwise_discretized_terms].
The gradient term requires the derivative of $f_b$ to be integrated over the volume $\Omega$.  

For the first case, we will consider a volumetric heat source that varies linearly across the body, which is given by 
\begin{equation}
f_b(x)=p_0+p_1x,
\end{equation}
with $p_0$ and $p_1$ the design parameters. Therefore, the derivative can be calculated as
\begin{equation}\label{eq:bodyLoad_example}
\begin{aligned}
\frac{\partial \hat{\text g}}{\partial\mathbf{p}} &= \int \text{N}\cdot\frac{\partial f_b(x)}{\partial\mathbf{p}}~\text{d}\Omega\\
&=
	\left[\int \text{N}\cdot \frac{\partial f_b}{\partial{p_0}}~\text{d}\Omega 
	, \int \text{N}\cdot \frac{\partial f_b}{\partial{p_1}}~\text{d}\Omega\right]^\top\\  
&= \left[\int\text{N}\cdot \left(1\right)~\text{d}\Omega
    ,\int\text{N}\cdot \left(x\right)~\text{d}\Omega\right]^\top.\\
\end{aligned}
\end{equation}

In the next force inversion case, we parameterize the intensity of $np$ point heat sources, where the heat source takes the form
\begin{equation}
f_b(x)=\sum_{i=1}^{np}\delta\left(x-x_i \right)p_i  \quad \text{for}~i=1 \dots np.
\end{equation}
The corresponding gradient term is given by
\begin{equation}\label{eq:pointLoad}
\begin{aligned}
 \\
\frac{\partial \hat{\text g}}{\partial p_i} &= \int \text{N}\cdot\frac{\partial f_b(x)}{\partial{p}_i}~\text{d}\Omega\\
& = \int \text{N}\cdot\delta\left(x-x_i \right)d\Omega \quad \text{for}~i=1 \dots np,
\end{aligned}
\end{equation}
which makes the gradient equal to one at the locations of the point loads.

Next, we will use force inversion to parameterize a Neumann boundary condition with the heat flux on the boundary being a function of the coordinates, $G(x)$, given by
\begin{equation}\label{eq:bcLoad}
\begin{aligned}
\frac{\partial\hat{\text{g}}}{\partial \boldsymbol{p}} & =  \frac{\partial\hat{g}}{\partial \hat{f}} \frac{\partial\hat{f}}{\partial G(x)} \frac{\partial G(x)}{\partial \boldsymbol{p}}\\
& = \frac{\partial\hat{f}}{\partial G(x)} \frac{\partial G(x)}{\partial \boldsymbol{p}} \\
& = \int_{\Gamma_R} \text{N}\cdot\frac{\partial G(x)}{\partial\mathbf{p}}~\text{d}\Gamma,\\
\end{aligned}
\end{equation}
where the derivative of $G(x)$ is now integrated over the boundary $\Gamma_R$. For instance, if we have a linearly varying heat flux 
\begin{equation}
G(x)=p_0 + p_1x,
\end{equation}
with $p_0$ and $p_1$ the design parameters, then
\begin{equation}\label{eq:neumannBC}
\begin{aligned}
\frac{\partial\hat{\text{g}}}{\partial \boldsymbol{p}} &= \left[ \frac{\partial\hat{\text{g}}}{\partial {p}_0}, \frac{\partial\hat{\text{g}}}{\partial {p}_1} \right]^\top  \\
 &=   \left[\int_{\Gamma_R} \text{N}\cdot \frac{\partial G(x)}{\partial p_0} ~\text{d}\Gamma
    ,\int_{\Gamma_R} \text{N}\cdot \frac{\partial G(x)}{\partial p_1} ~\text{d}\Gamma\right]^\top \\
&= \left[\int_{\Gamma_R} \text{N}~\text{d}\Gamma
    ,\int_{\Gamma_R} \text{N}x~\text{d}\Gamma\right]^\top.\\
\end{aligned}
\end{equation}

The above force inversion examples, [eq:bodyLoads] and [eq:neumannBC], are all linear optimization problems where the parameter being optimized does not show up in the derivative term.  Linear optimization problems are not overly sensitive to the location of measurement points or the initial guesses for the parameter being optimized, making them easy to solve.  In the following we parameterize a Gaussian body force given by 
\begin{equation}\label{eq:gaussOpt}
\begin{aligned}
f_b(x) = a \cdot \exp{\left( -\frac{(x-b)^2}{2c^2} \right)},
\end{aligned}
\end{equation}
where $a$ is the height or intensity of the Gaussian curve, $b$ is the location of the peak of the curve, and $c$ is the standard deviation of the curve or its width.  Parameterizing a Gaussian curve can result in a linear or nonlinear optimization problem depending on which parameter is being optimized.  Parameterizing this function for the height, $a$, results in the following linear optimization problem derivative
\begin{equation}\label{eq:gaussOpt_a}
\begin{aligned}
\frac{\partial\hat{\text{g}}}{\partial \boldsymbol{p}} 
& = \frac{\partial\hat{\text{g}}}{\partial a}  = \int \text{N}\cdot\frac{\partial f_b(x)}{\partial a}~\text{d}\Omega\\
 &= \int \text{N}\cdot  \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}~\text{d}\Omega,
\end{aligned}
\end{equation}
where the parameter $a$ being optimized does not show up in the derivative term.  
However, if we try to parameterize for the location of the peak of the Gaussian curve, $b$, we get the following nonlinear optimization problem derivative
\begin{equation}\label{eq:gaussOpt_b}
\begin{aligned}
\frac{\partial\hat{\text{g}}}{\partial \boldsymbol{p}} & = \frac{\partial\hat{\text{g}}}{\partial b} = \int \text{N}\cdot\frac{\partial f_b(x)}{\partial b}~\text{d}\Omega\\
&=\int \text{N}\cdot \frac{a(x-b)}{c^2} \exp{\left( -\frac{(x-b)^2}{2c^2} \right)}~\text{d}\Omega,
\end{aligned}
\end{equation}
where the parameter $b$ remains in the derivative term.  Optimizing for the peak location of a Gaussian heat source, [eq:gaussOpt_b],  will be a much more difficult problem to solve than [eq:gaussOpt_a] and convergence will be dependent on the initial guesses given for $b$ and the locations where measurement data is taken. 

[comment]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivative for Convective Boundary Conditions id=sec:robinInv

In this section, the convective heat transfer coefficient, $h$, is considered as a parameter for the convection boundary condition given in \eq{eq:robin_bc_types}.  This is a material inversion problem with integral limited to the boundary, $\Gamma_R$.  The boundary condition is given by $G(T)=h \left(T - T_{\infty}\right)$ on $\Gamma_R$.  The PDE derivative term is given by
\begin{equation}\label{eq:convectiveBC}
\begin{aligned}
\frac{\partial\hat{\text{g}}}{\partial \boldsymbol{p}} &= \frac{\partial\hat{\text{g}}}{\partial h}
= \int_{\Gamma_R} \text{N}\cdot\frac{\partial G(x)}{\partial h}~\text{d}\Gamma\\
 &= \int_{\Gamma_R} \text{N}\cdot \left(T-T_{\infty} \right)~\text{d}\Gamma.
\end{aligned}
\end{equation}
This derivative requires the solution from the forward problem $T$ to be included in the integral over the boundary $\Gamma_R$ which again results in a nonlinear optimization problem.  

[comment]: <> (% ----------------------------------------------------------------------------------------------------------%)

## PDE Derivative for Material Inversion id=sec:material_inversion

In [#sec:forceInv] and [#sec:robinInv], the design parameters only exist in the load terms. Therefore, 
the derivatives of $g(x)$ with respect to the design parameters are only related to the form of the load function, not the Jacobian.  In this section, we consider cases where the design parameters exist in the Jacobian term, which make the derivative calculation more convoluted. One such example is the material inversion, where we identify the thermal conductivity ($\kappa$) through experimentally measured temperature data points. Here, the derivative of $g(x)$ is taken with respect to $\kappa$. This requires the derivative of $\textbf{K}$ in \eq{eq:discretePDE} leading to
\begin{equation}\label{eq:kappa}
\begin{aligned}
\frac{\partial \hat{\text g}}{\partial\mathbf{p}} &=  \int\nabla\text{N}^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla\text{N}~\text{d}\Omega \cdot\hat{\text T}\\
    &=  \int\nabla\text{N}^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla T~\text{d}\Omega,
\end{aligned}
\end{equation}
where $\nabla\text{N}\hat{T}=\nabla T$ was used in the last line.  The gradient term given by [eq:kappa] is multiplied by the adjoint variable $\lambda$ in [eq:adjointGrad], giving

\begin{equation}\label{eq:kappaLambda}
\begin{aligned}
\lambda\frac{\partial \hat{\text g}}{\partial\mathbf{p}} &=  \hat{\lambda}^{\top}\cdot\int\nabla\text{N}^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla\text{N}~\text{d}\Omega \cdot\hat{\text T}\\
    &=  \int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{{\partial\mathbf{p}}}\cdot\nabla T~\text{d}\Omega,
\end{aligned}
\end{equation}
where  $\hat{\lambda}^{\top}\cdot\nabla\text{N}^{\top}=\left[\nabla\text{N}\cdot\hat{\lambda}\right]^{\top}=\left[\nabla\lambda\right]^{\top}=\nabla\lambda^{\top}$ is used in the last line.  Computing the integral in [eq:kappaLambda] requires an inner product of the gradients of the adjoint and forward variables. This is much simpler to compute than the integral in [eq:kappa] which requires the multiplication of nodal variables with elemental shape function gradients.  Material inversion is also a nonlinear optimization problem since $T$ shows up in the derivative, making the derivative dependent on the solution to the forward problem.

This also works for temperature dependent thermal conductivity
\begin{equation}
\kappa\left(T\right)=\alpha T^2+\beta T,
\end{equation}
where $\alpha$ and $\beta$ are the design parameters. The resultant derivative is then
\begin{equation}\label{eq:TdependentKappa}
\begin{aligned}
\lambda\frac{\partial \hat{g}}{\partial\mathbf{p}}= 
    \lambda\left[ \frac{\partial \hat{g}}{\partial\alpha},\frac{\partial \hat{g}}{\partial\beta}\right]^\top
    &= \left[\int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{\partial\alpha}\cdot\nabla T~\text{d}\Omega
        ,\int\nabla\lambda^{\top} \cdot \frac{\partial \kappa}{\partial\beta}\cdot\nabla T~\text{d}\Omega\right]^\top\\
    &= \left[\int\nabla\lambda^{\top} \cdot \left(T^2\right) \cdot\nabla T~\text{d}\Omega,\int\nabla\lambda^{\top} \cdot \left(T\right) \cdot\nabla T~\text{d}\Omega\right]^\top.
\end{aligned}
\end{equation}

!bibtex bibliography
