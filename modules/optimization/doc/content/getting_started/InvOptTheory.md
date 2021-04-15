# Inverse Optimization Theory

Inverse optimization is a mathematical framework to infer model parameters by minimizing the misfit between the experimental and simulation observables.  In this work, our model is a partial differential equation (PDE) describing the physics of the experiment.  We solve our physics PDE using the finite element method as implemented in MOOSE.  The physics of our problem constrains our optimization algorithm.  A PDE-constrained inverse optimization framework is formulated as an abstract optimization problem (ref2):

\begin{equation}\label{eq:optimization}
   \min_{\bm{p}} J\left(\bm{u},\bm{p}\right);\quad~\textrm{subject to}~\bm{g}\left(\bm{u},\bm{p}\right)=\bm{0}
\end{equation}

where $J(\bm{u},\bm{p})$ is our objective function, which is a scalar measure of the misfit between experimental and simulated responses, along with any regularization ref12.  The constraint, $\bm{g}\left(\bm{u},\bm{p}\right)=\bm{0}$, consists of the PDEs governing the multiphysics phenomena simulated by MOOSE (e.g. coupled heat and elasticity equations), $\bm{p}$ contains model parameters (e.g. material properties or loads) and $\bm{u}$ contains simulated responses (e.g. temperature and displacement fields).  The equations in [eq:optimization] appear simple on the outset but are extremely difficult to solve. The solution space can span millions of degrees of freedom and the parameter space can also be very large. Finally, the PDEs can be highly nonlinear, time-dependent and tightly coupling complex phenomena across multiple physics.

Optimization problems can be solved using either global (gradient-free) or local (gradient-based) approaches (ref1). Global approaches require a large number of iterations compared to gradient-based approaches (e.g. conjugate gradient or Newton-type methods), making the latter more suitable to problems with a large parameter space and computationally expensive models.  Isopod uses the `PETSc TAO` optimization library to solve [eq:optimization]. See [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) for a list of `TAO` gradient and gradient-free optimization solvers available in isopod.  Optimization libraries like `TAO` require access to functions for computing the objective ($J$), gradient $\left(\mathrm{d}J/\mathrm{d}\bm{p}\right)$ and Hessian $\left(\mathrm{d}^2J/\mathrm{d}\bm{p}^2\right)$ or a function to take the action of the Hessian on a vector.  An objective function measuring the misfit or distance between the simulation and experimental data usually has the form

\begin{equation}\label{eq:objective}
J(\bm{u}, \bm{p}) = \frac{1}{2}\sum_i \left(\bm{u}_i - \bm{\bar u}_i\right)^2 + \frac{\rho}{2}\sum_i \bm{p}^2 
\end{equation}

where the first integral is an L$_2$ norm or euclidean distance between the experimental solution, $\bm{\bar u}$, and the simulated solution, $\bm{u}$.  The second integral provides [Tikhonov](https://en.wikipedia.org/wiki/Tikhonov_regularization) regularization on the parameters, $\bm{p}$, for ill-posed problems where $\rho$ controls the amount of regularization.  Other types of regularization may also be used.

Gradient-free optimization solvers only require a function to solve for the objective given in [eq:objective].  Solving for the objective only requires solving a forward problem to determine $\bm{u}$ and then plugging that into [eq:objective] to determine $J$.  The forward problem is defined as the FEM model of the experiment which the analyst should have already made before attempting to perform optimization.  The parameters that go into the forward problem (e.g. pressure distributions on sidesets or material properties) are adjusted by the optimization solver and the forward problem is recomputed.  This process continues until $J$ is below some user defined threshold. The basic gradient-free solver available in `TAO` is the simplex or [Nelder-Mead](https://en.wikipedia.org/wiki/Nelder%E2%80%93Mead_method) method.  Gradient-free optimization solvers are robust and straight-forward to use.  Unfortunately, their computational cost scales exponentially with the number of parameters.  When the forward model is a computationally expensive FEM model, gradient-free approaches quickly become computationally expensive.

Gradient-based optimization algorithms require fewer iterations but require functions to solve for the gradient vector and sometimes Hessians matrix.  `TAO` has `petsc_options` to evaluate finite difference based gradients and Hessians by solving the objective function multiple times with perturbed parameters, which requires multiple solves of the forward problem too.  Finite difference gradients and Hessians are good for testing an optimization approach but become computationally expensive for realistic problems.

Given the large parameter space, we resort to the adjoint method for gradient computation; unlike finite difference approaches, the computational cost of adjoint methods is independent of the number of parameters (ref18).  In the adjoint method, the gradient, i.e. the total derivative $\mathrm{d}J/\mathrm{d}\bm{p}$, is computed as,

\begin{equation}\label{eq:adjointGrad}
\frac{\mathrm{d}J}{\mathrm{d}\bm{p}} = \frac{\partial J}{\partial\bm{p}}+\bm{\lambda}\frac{\partial\bm{g}}{\partial\bm{p}}
\end{equation}

where $\partial J/\partial\bm{p}$ accounts for the regularization in [eq:objective] and $\bm{\lambda}$ is the adjoint variable solved for from the adjoint equation

\begin{equation}\label{eq:adjoint}
\left(\frac{\partial\bm{g}}{\partial\bm{u}}\right)^{\intercal} \bm{\lambda}= \left(\frac{\partial J}{\partial\bm{u}}\right)^{\intercal}
\end{equation}

where $\left(\partial\bm{g}/\partial\bm{u}\right)^{\intercal}$ is the adjoint of the Jacobian for the original forward problem, $\bm{g}$, and $\left(\partial J/\partial\bm{u}\right)^{\intercal}$ is a body force like term that accounts for the misfit between the computed and experimental data.  Thus, the solution to [eq:adjoint] has the same complexity as the solution to the forward problem.  

The remaining step for evaluating the derivative of the PDE in [eq:adjointGrad] is to compute $\partial\bm{g}/\partial\bm{p}$, the derivative of the PDE with respect to the parameter vector.  The form this term takes is dependent on the physics (e.g. mechanics or heat conduction) and the parameter being optimized (e.g. force inversion versus material inversion).  In what follows, we will derive the adjoint equation for steady state heat conduction and the gradient term for both force and material inversion.  

## Adjoint Problem for Steady State Heat Conduction

In this section, we are interested in solving the following PDE-constrained optimization problem from [eq:optimization] for steady state [heat conduction](/heat_conduction/index.md):

\begin{equation} \label{eqn:optimization_problem}
	\begin{aligned}
	& \min_{\bm{p}}
	& & J\left(T,\bm{p}\right) = \frac{1}{2} \sum_{i=1}^{N} \left( T_i - \bar{T}_i \right)^2 \\
	& \text{subject to}
	& & g\left(T,\bm{p}\right)=\nabla \cdot \kappa \nabla T + f_b =0, & \text{in}~\Omega \\
\end{aligned}
\end{equation}
where $J$ is the objective function from [eq:objective] without regularization, $f_b$ is the distributed heat flux, $T$ is the experimental temperature field being compared to our simulation temperature at discrete locations, $\bar{T}_i$.  Other forms for the objective function are possible such as different norms or different types of measurements that may require integration over a line or volume.

We also have the following boundary conditions for our PDE,

\begin{equation} \label{eqn:optimization_bcs}
	\begin{aligned}
	& & & T = T_D, &\text{on}~\Gamma_D, \\
	& & & \left( \kappa \nabla T \right) \cdot \boldsymbol{n} = G\left(T\right), &\text{on}~\Gamma_R. \\
\end{aligned}
\end{equation}

where $\boldsymbol{n}$ is the normal vector, $\Gamma_D$ is the Dirichlet boundary, and $\Gamma_R$ is the Robin or mixed boundary. Common  cases for $G\left(T\right)$ are:

\begin{equation}
\begin{aligned}\label{eq:robin_bc_types}
   &\text{Neumann:     }~G\left(T\right) = G = f_n \\
   &\text{Convection:  }~G\left(T\right) = h (T - T_{\infty}) \\
\end{aligned}
\end{equation}
where $h$ is the heat transfer coefficient, see [ConvectiveHeatFluxBC.md], and $f_n$ is independent of $T$.
The objective function can also be expressed in a integral form for the case of $N$ point measurements as follows
\begin{equation}  \label{eqn:objective_integral}
	J = \frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \bar{T}_i \right)^2\text{d}\Omega.
\end{equation}


We take the equivalent, variational approach to derive the adjoint.  Thus, the Lagrangian of this problem is
\begin{equation} \label{eqn:lagrangian}
\begin{aligned}
	\mathcal{L}(T, \bm{p}, \lambda) & = J + \int \left(  \nabla \cdot \kappa \nabla T + f_b  \right) \lambda~\text{d}\Omega \\
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
	\mathcal{L}(T, \bm{p}, \lambda) =& \mathcal{A}(T, \bm{p}, \lambda) +\mathcal{B}(T, \bm{p}, \lambda) \\
	 \mathcal{A}(T, \bm{p}, \lambda)=&\frac{1}{2} \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \bar{T} \right)^2\text{d}\Omega 
	 	+ \int \left(  f \lambda \right) ~\text{d}\Omega  
	 	+ \int \left(\nabla\cdot \kappa \nabla \lambda\right)T ~\text{d}\Omega\\
	 \mathcal{B}(T, \bm{p}, \lambda)=&  \int\left[ \lambda \left( \kappa \nabla T \right)\cdot \boldsymbol{n} 
	   	- T\left( \kappa \nabla \lambda  \right) \cdot \boldsymbol{n}\right]~\text{d}\Gamma.
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
    			-\int_{\Gamma_D} T_{o}\kappa\nabla\lambda\cdot\boldsymbol{n}~\text{d}\Gamma\\
\end{aligned}
\end{equation}
where $T_o$ is the prescribed temperature on the Dirichlet boundary, $\Gamma_D$.  The variation of $\mathcal{L}$ with respect to $T$ is then given by $\delta\mathcal{L}=\delta\mathcal{A}+\delta\mathcal{B}$ where the variation of the body terms with respect to $T$ are given by
\begin{equation}\label{eqn:delta_bodyLagrangian}
\begin{aligned}
\delta\mathcal{A}=& \sum_{i=1}^{N} \int \delta(x - x_i) \left( T - \bar{T} \right)\delta T\text{d}\Omega + \int \left(\nabla\cdot \kappa \nabla \lambda \right) \delta T ~\text{d}\Omega\\
				=&\int \left(\left(\nabla\cdot \kappa \nabla \lambda \right) +  \sum_{i=1}^{N} \delta(x - x_i) \left( T - \bar{T} \right) \right)\delta T ~\text{d}\Omega
\end{aligned}
\end{equation}
and the variation of $\mathcal{B}$ with respect to $T$ is given as
\begin{equation}\label{eqn:delta_bcLagrangian}
\begin{aligned}
\delta\mathcal{B}=&\int_{\Gamma_R}\lambda\frac{\text{d}G(T)}{\text{d}T}\delta T \text{d}\Gamma
                    		-\int_{\Gamma_R}\left(\kappa\nabla\lambda\cdot\boldsymbol{n}\right)\delta T\text{d}\Gamma
                           	+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma\\
=&\int_{\Gamma_R}\left(\lambda h -\left(\kappa\nabla\lambda\cdot\boldsymbol{n}\right)\right)\delta T\text{d}\Gamma
    				+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma
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
    	+\int_{\Gamma_D}\lambda\left(\kappa\nabla\delta T\cdot\boldsymbol{n}\right)~\text{d}\Gamma
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

## PDE Derivative for Force Inversion

In this section we will present force inversion for steady state heat conduction given in [eqn:optimization_problem].  For all of these examples, measurement data is taken at specific locations where the objective function can be represented by [eqn:objective_integral].  We will present the discrete forms of our PDE and its derivative which most closely matches the implementation that will be used in MOOSE.  
The discrete form of the PDE constraint for steady state heat conduction in [eqn:optimization_problem], $\hat{g}$, is given as
\begin{equation}\label{eq:discretePDE}
\begin{aligned}
\hat{g}&=\bm{K}\hat{T}-\hat{f}=0\\
\bm{K}&=\int\left(\nabla\text{N}\right)^{\intercal} \cdot \kappa\cdot\nabla\text{N}~\text{d}\Omega\\
\hat{f}&=\int\text{N} f(x)~\text{d}\Omega\\
T(x)&\approx \text{N}\hat{T}\\
\nabla T&\approx \nabla\text{N}\hat{T}\\
\end{aligned}
\end{equation}
where $\bm{K}$ is the Jacobian, $\hat{T}$ and $\hat{f}$ are the discretized nodal temperatures and forces that can be interpolated using the element's shape functions $N$ to approximate the continuous fields, $T(x)$ and $f(x)$.  We are assuming a Galerkin form for our discretized PDE by making our test and trial functions both $\text{N}$.

We will first consider load parameterization of body loads, $f_b$, where the gradient is given as
\begin{equation}\label{eq:bodyLoads}
\begin{aligned}
\hat{f}_b &= \int \text{N}\cdot f_b(x)~\text{d}\Omega\\
\frac{\partial \hat{g}}{\partial\bm{p}} &= \frac{\partial \hat{f}_b}{\partial\bm{p}} 
	= \int \text{N}\cdot\frac{\partial f_b(x)}{\partial\bm{p}}~\text{d}\Omega\\
\end{aligned}
\end{equation}

The gradient term requires the derivative of $f_b$ to be integrated over the volume $\Omega$.  For the first case, we will consider a heat source that varies linearly across the body given by
\begin{equation}\label{eq:bodyLoad}
\begin{aligned}
f_b(x)&=p_0+p_1x\\
\hat{f}_b &= \int \text{N}\cdot\left( p_0+p_1x\right)~\text{d}\Omega\\
\frac{\partial \hat{g}}{\partial\bm{p}} = \frac{\partial \hat{f}_b}{\partial\bm{p}} &=
	\int \text{N}\cdot  \left(  \frac{\partial f}{\partial{p_0}}+\frac{\partial f}{\partial{p_1}}  \right)~\text{d}\Omega\\  
&= \int\text{N}\cdot \left(x+1\right)~\text{d}\Omega\\
\end{aligned}
\end{equation}
In second case, we parameterize the intensity of point heat sources, where the gradient term is given by
\begin{equation}\label{eq:pointLoad}
\begin{aligned}
\hat{f}_b &= \sum_{i=1}^N\int \text{N}\cdot\delta\left(x-x_i \right)p_i d\Omega\\
\frac{\partial \hat{f}_b}{\partial\bm{p}} &= \sum_{i=1}^N\int \text{N}\cdot\delta\left(x-x_i \right)d\Omega\\
\end{aligned}
\end{equation}
which makes the gradient equal to one at the locations of the point loads.
We can also parameterize 
For the next case, we will parameterize a heat flux boundary condition that is a function of the coordinates, $f(x)$, given by
\begin{equation}\label{eq:neumannBC}
\begin{aligned}
\hat{f}_n &= \int_{\Gamma_R}\text{N}\cdot f(x)~\text{d}\Gamma\\
\frac{\partial \hat{f}_n}{\partial\bm{p}} &= \int_{\Gamma_R}\text{N}\cdot\frac{\partial f(x)}{\partial p}~\text{d}\Gamma\\
\end{aligned}
\end{equation}

where the derivative of $f_n$ needs to be integrated over the surface $\Gamma_R$. For instance, if we have a linearly varying heat flux, [eq:neumannBC] would be
\begin{equation}\label{eq:neumannBC}
\begin{aligned}
f(x)&=p_1x+p_0\\
\frac{\partial \hat{g}}{\partial\bm{p}} &= \frac{\partial \hat{g}}{\partial{p_0}}+\frac{\partial \hat{g}}{\partial{p_1}} \\
&= \int_{\partial\Omega_R} \text{N}\cdot \left(x+1\right)~\text{d}S\\
\end{aligned}
\end{equation}
The optimization problem becomes nonlinear when the parameters do not drop out of the gradient term, for instance
\begin{equation}\label{eq:neumannBC}
\begin{aligned}
f(x)&=p_1^2x+p_0\\
\frac{\partial \hat{g}}{\partial\bm{p}} &= \int_{\partial\Omega_R} \text{N}\left(p_1x+1\right)~\text{d}S\\
\end{aligned}
\end{equation}
where now, the gradient is still a function of the parameter $p_1$, making this much more difficult for the optimization solver.

## PDE Derivative for Convective Boundary Conditions

For this case we are parameterizing the convective heat transfer coefficient, $h$, for the convection boundary condition from [eq:robin_bc_types] where $G(T)=h \left(T - T_{\infty}\right)$.  This is a material inversion problem where the integral is only over the boundary.  The gradient term is given by
\begin{equation}\label{eq:convectiveBC}
\begin{aligned}
\hat{f}_c &= \int_{\partial\Omega_R} \text{N}\cdot h \left(T - T_{\infty}\right)~\text{d}S\\
\frac{\partial \hat{g}}{\partial\bm{p}} &= \frac{\partial \hat{f}_c}{\partial h} = \int_{\partial\Omega_R} \text{N}\cdot \left(T-T_{\infty} \right)~\text{d}S\\
\end{aligned}
\end{equation}

This requires the solution from the forward problem $T$ to be included in the integral over the boundary $\partial \Omega_R$.

fixme  Do these need to be $\hat{T}\cdot N$ instead of $T$.  I'm not sure.  

## PDE Derivative for Material Inversion

In material inversion, $\kappa$ is being parameterized.  Material inversion requires the derivative of $\text{K}$ in [eq:pointLoad] leading to
\begin{equation}\label{eq:kappa}
\begin{aligned}
\frac{\partial \hat{g}}{\partial\bm{p}} &= \frac{\partial \hat{g}}{\partial \kappa}=\frac{\partial}{\partial\kappa}\left(\text{K}\hat{T}\right)\\
&=\frac{\partial }{\partial\kappa} \left(\int_{\Omega} \nabla^{\intercal} \text{N} \cdot \kappa \nabla \text{N}\hat{T}~\text{d}\Omega\right)\\
&=\int_{\Omega} \nabla^{\intercal} \text{N} \cdot \nabla\hat{T}~\text{d}\Omega\\
\end{aligned}
\end{equation}
where $\nabla\text{N}\hat{T}=\nabla\hat{T}$ was used in the last line.  Material inversion is a nonlinear optimization problem since $\hat{T}$ shows up in our gradient term, making the gradient dependent on the solution to the forward problem.

This also works for temperature dependent thermal conductivity where $\alpha$ and $\beta$ are the design parameters:
\begin{equation}\label{eq:TdependentKappa}
\begin{aligned}
\kappa\left(T\right)&=\alpha T^2+\beta T\\
\frac{\partial \hat{g}}{\partial\bm{p}} &= \frac{\partial\text{K}}{\partial\kappa}\hat{T} = \frac{\partial\text{K}}{\partial\alpha}\hat{T} + \frac{\partial\text{K}}{\partial\beta}\hat{T}\\
&=\int_{\Omega} \nabla^{\intercal} \text{N} \cdot \left( T^2 + T \right) \nabla \text{N}\hat{T}~\text{d}\Omega\\
\end{aligned}
\end{equation}

fixme  Do these need to be $\hat{T}\cdot N$ instead of $T$.  I'm not sure.  
