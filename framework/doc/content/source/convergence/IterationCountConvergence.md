# IterationCountConvergence

This [Convergence](Convergence/index.md) specifies:

- $\ell_\text{max}$, the maximum number of iterations,
  via [!param](/Convergence/IterationCountConvergence/max_iterations), and
- $\ell_\text{min}$, the minimum number of iterations,
  via [!param](/Convergence/IterationCountConvergence/min_iterations).

If [!param](/Convergence/IterationCountConvergence/converge_at_max_iterations)
is set to `true`, then the solve will converge when $\ell = \ell_\text{max}$
instead of diverge.

!alert note title=Clarification of starting iteration index
Some iteration loops, such as nonlinear iteration, begin with a convergence check with $\ell = 0$ before taking any "step"; then after the first *step*, it has $\ell = 1$. Other iteration loops such as the fixed point iteration loop do not have such an initial convergence check, so in that case $\ell = 0$ corresponds to the end of the first step. Choose [!param](/Convergence/IterationCountConvergence/max_iterations) and [!param](/Convergence/IterationCountConvergence/min_iterations) accordingly.

## Design

Other `Convergence` objects may inherit from this class and override
`checkConvergenceInner(iter)` instead of the usual `checkConvergence(iter)`,
to inherit the iteration bounds. An example is [/PostprocessorConvergence.md].

!syntax parameters /Convergence/IterationCountConvergence

!syntax inputs /Convergence/IterationCountConvergence

!syntax children /Convergence/IterationCountConvergence
