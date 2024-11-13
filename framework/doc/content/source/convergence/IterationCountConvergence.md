# IterationCountConvergence

This [Convergence](Convergence/index.md) specifies:

- $\ell_\text{max}$, the maximum number of iterations,
  via [!param](/Convergence/IterationCountConvergence/max_iterations), and
- $\ell_\text{min}$, the minimum number of iterations,
  via [!param](/Convergence/IterationCountConvergence/min_iterations).

If [!param](/Convergence/IterationCountConvergence/converge_at_max_iterations)
is set to `true`, then the solve will converge when $\ell = \ell_\text{max}$
instead of diverge.

## Design

Other `Convergence` objects may inherit from this class and override
`checkConvergenceInner(iter)` instead of the usual `checkConvergence(iter)`,
to inherit the iteration bounds. An example is [/PostprocessorConvergence.md].

!syntax parameters /Convergence/IterationCountConvergence

!syntax inputs /Convergence/IterationCountConvergence

!syntax children /Convergence/IterationCountConvergence
