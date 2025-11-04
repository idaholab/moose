# PseudoTimestep

The methods implemented in this postprocessor compute timesteps adaptively using Pseudo Transient Continuation strategies. This allows for steady-state problems to be solved via a time marching approach to reach the final state rapidly.
As a reminder, the time marching approach to computing steady states reframes a problem in discrete form $M\frac{\partial \mathbf u}{\Delta t}=F(\mathbf u)$, where $M$ is the mass matrix and $F()$ an operator gathering the right-hand side discrete terms.
Using implicit Euler, the problem reads

\begin{equation}
M\frac{\mathbf u_{n+1}-\mathbf u_n}{\Delta t}=F(\mathbf u_{n+1})
\end{equation}

To be consistent with the literature the current implementation has been tested only on implicit Euler.

## Overview

This object computes a timestep to accelerate the convergence to steady-state using pseudo-transient continuation.
The change in timestep is determined by the steady-state residual behavior from one iteration to another, i.e., small changes in residual indicate larger timesteps are allowed. In contrast, significant changes in the residual indicate a timestep decrease is necessary.
Following [!citep](bucker2009cfl), we recognize and implement three methods.

The user must make a method choice, between `SER`, `EXP` and `RDM`, which are implemented as listed below.
All methods require a parameter [!param](/Postprocessors/PseudoTimestep/alpha), which controls how sensitive the timestep should be with respect to residual changes, and [!param](/Postprocessors/PseudoTimestep/initial_dt) to provide a first timestep length.
If nothing is known about the problem we recommend `initial_dt = 1` and `alpha = 2`, keeping in mind that a high [!param](/Postprocessors/PseudoTimestep/alpha) corresponds to a higher sensitivity to residual changes. More specific choices for fluid dynamics problems are available in [!citep](bucker2009cfl) or [!citep](ceze2013pseudo). The parameter [!param](/Postprocessors/PseudoTimestep/alpha) is always larger than 0, noting that for some versions of Pseudo Timestep Continuation methods it can be lower than 1. We refer the user to the literature, or to perform a preliminary study for their specific problem.

Methods supported include:

- +Switched evolution relaxation (SER)+

  \begin{equation}
  \Delta t_k =\Delta t_{k-1} \cdot \bigg(\frac{R_{k-\ell}}{R_{k}}\bigg)^{\alpha}
  \end{equation}

  where $\alpha$ is a user chosen parameter. The l2-norm of the steady-state residual at step $k$ is $R_k$, and the residual at $\ell$ iterations before is denoted as $R_{k-\ell}$. To set a number of previous iterations corresponding to $\ell$ the user can prescribe an integer value for the parameter [!param](/Postprocessors/PseudoTimestep/iterations_window).

- +Residual Difference Method (RDM)+

  \begin{equation}
  \Delta t_k =\Delta t_{k-1} \cdot \alpha^{\frac{R_{k-1}-R_k}{R_{k-1}}}
  \end{equation}

  This implementation is the `RDM` method variant as found in [!citep](ceze2013pseudo), other variants are available in e.g. [!citep](bucker2009cfl).

- +Exponential progression (EXP)+

  \begin{equation}
  \Delta t_k =\Delta t_{0} \cdot \alpha^k
  \end{equation}

  where $\alpha$ is a user chosen parameter, $k$ is the current iteration step.

As noted also in [!citep](bucker2009cfl) the EXP method has an infinite growth, so for this method a [!param](/Postprocessors/PseudoTimestep/max_dt) parameter may be recommended. If no [!param](/Postprocessors/PseudoTimestep/max_dt) is provided by the user then infinite growth of the timestep is not bounded and the user will be informed by a message at the console. Ideally this method is used in conjunction with a steady state detection, i.e. setting [!param](/Executioner/Transient/steady_state_detection) to `true` and a [!param](/Executioner/Transient/steady_state_tolerance).

## Example Input File Syntax

!listing test/tests/postprocessors/pseudotimestep/fv_burgers_pseudo.i
    block=Postprocessors

!syntax parameters /Postprocessors/PseudoTimestep

!syntax inputs /Postprocessors/PseudoTimestep

!syntax children /Postprocessors/PseudoTimestep

!bibtex bibliography
