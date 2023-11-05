# ResidualConvergence

By default, MOOSE checks convergence using relative and absolute criteria. Once the residual drops
below either an absolute tolerance or the residual divided by the initial residual for the current
timestep drops below a relative tolerance, the solution is considered converged.

### +Solver convergence criteria parameters+

Parameters for setting absolute convergence, relative convergence etc. are usually set in the `Executioner` block. However, now they can also prescribed in the `Convergence` block.

We shall provide a few guidelines for setting convergence parameters. In the following presume we have a partial differential equation, given as 
$\mathcal{P}(\mathbf u)=f$, and subsequent residual $\mathcal R(\mathbf u)=\mathcal P(\mathbf u)-f=0$. Convergence with respect to a tolerance $\tau$, implies $|\mathcal R (\mathbf u)|<\tau$, while divergence is encountered when the residual cannot decrease below the value of $\tau$.

If we consider an iterative process to determine the solution $\mathbf u$, we have a set of intermediary solutions $\mathbf u_i$ required to verify the equation $\mathcal R(\mathbf u)\approx 0$, but do not meet the required tolerance $\tau$.

#### 1. Absolute and relative tolerances

- +Absolute Tolerance (`abs_tol`)+: This is a parameter that determines the threshold at which the residual norm of the solution is deemed small enough in absolute terms. For a system of equation this translates into $|\mathbf u_{i+1}-\mathbf u_i|<\tau$.
 
- +Relative Tolerance (`rel_tol`)+: This parameter sets the threshold for the residual norm in relation to the norm of the right-hand side of the equation. For a system of equation this translates into $|\mathbf u_{i+1}-\mathbf u_i|/|\mathbf u_{i+1}|<\tau$.


Considering that nonlinear systems are ultimately solved via linearization the user should append `l_` for linear systems, or `nl_` for nonlinear ones, on a per case base.

#### 2. Choosing appropiate `abs_tol` and `rel_tol`

- To start, apply the same values for both `abs_tol` and `rel_tol` in both preconditioned and non-preconditioned systems. This allows for a straightforward comparison under similar stopping criteria.
- +Preconditioning+ aims to improve the numerical properties of an iterative solver (specifically the condition number). This often results in the preconditioned system converging more quickly or requiring fewer iterations. However, preconditioning modifies the scale and the properties of the problem, which can affect the scale of the residuals.
- The user must make sure that `abs_tol` and `rel_tol` are set in a manner that mirrors the +scaling+ of the problem. If preconditioning significantly changes the scale (which is often the case), the user needs to adjust these tolerances to accommodate the changes.
- The user should +experiment+ with different settings of `abs_tol` and `rel_tol` to observe their effect on convergence and the quality of the solution. To make a fair comparison, one should ensure that the solver’s behavior (in terms of convergence and accuracy) is similar under both settings.

#### Other stopping criteria

Presuming the user gathered enough knowledge about the solver behaviour and the nature of the problem, addtional stopping requirements can be added, such as `nl_max_its` which instructs after how many iterations to abort the solver, or `nl_abs_step_tol`, which indicates what tolerance to be accepted at each solver step.  

## Example input syntax

!listing test/tests/convergence/residual_convergence/diffusion_convergence.i block=Convergence

!listing test/tests/convergence/residual_convergence/diffusion_convergence.i block=Executioner
where the [!param](/Executioner/nonlinear_convergence) indicates the convergence type to be `ResidualConvergence` and additional parameters. Curently convergence specific parameters can be still specified in the Executioner block.

!syntax parameters /Convergence/ResidualConvergence

!syntax inputs /Convergence/ResidualConvergence

!syntax children /Convergence/ResidualConvergence
