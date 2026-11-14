# Crossflow damping on EBR-II SHRT-17

This page records how the crossflow damping of the monolithic algorithm behaves on a real case. The
method itself is described in [Crossflow damping](subchannel_theory.md#monolithic); here only the
measured behavior is reported. The case is the EBR-II SHRT-17 steady state,
`test/tests/problems/SFR/EBR-II/XX09_SS_SHRT17.i`, a 61-pin wire-wrapped assembly at 486.2 kW with
50 axial cells, solved with [!param](/Problem/TriSubChannel1PhaseProblem/P_tol) of $10^{-4}$ and
[!param](/Problem/TriSubChannel1PhaseProblem/T_tol) of $10^{-5}$.

## Why the damping needs both of its parts

!! Intentional comment to provide extra spacing

The damping term is $\mathbf K^{\ell}(\mathbf W^{\ell+1} - \mathbf W^{\ell})$, a diagonal shift of
the cross-momentum block together with a matching right-hand-side term. Removing either part
degrades the solve:

| Damping | Temperature iterations | Pressure drop |
| - | - | - |
| Decaying, with right-hand-side compensation (the default) | 11 | 41757 Pa |
| Fixed, with right-hand-side compensation | about 4 times as many | unchanged |
| Fixed, without compensation | — | 0.5% different |

The compensation is what makes the converged solution independent of $\mathbf K$, since the term
vanishes at the fixed point for any $\mathbf K$. Without it the added resistance $\mathbf K\,\mathbf
W$ survives at convergence and biases the crossflow toward zero in every axial cell, so the error
grows with axial refinement instead of vanishing. The decay of $\mathbf K$ is what keeps the
iteration fast: a large $\mathbf K$ conditions the early solves well but slows the outer iteration
near convergence.

## Choosing the initial pseudo-time step

!! Intentional comment to provide extra spacing

The initial step [!param](/Problem/TriSubChannel1PhaseProblem/crossflow_pseudo_time_step),
$\Delta\tau_0$, sets how strongly the first outer iterations are damped. Sweeping it over five
orders of magnitude on this case:

| $\Delta\tau_0$ (s) | Temperature iterations | Coupled solve retries | Pressure drop (Pa) |
| - | - | - | - |
| $5\times10^{-6}$ | 47 | 0 | 41756.8 |
| $5\times10^{-5}$ | 17 | 0 | 41756.9 |
| $5\times10^{-4}$ (default) | 11 | 0 | 41757.0 |
| $5\times10^{-3}$ | 9 | 0 | 41757.1 |
| $10^{-1}$ | 9 | 0 | 41757.2 |

Two things to read from this. First, the converged solution does not depend on $\Delta\tau_0$: the
pressure drop varies by 0.4 Pa out of 41.8 kPa, which is the outer-iteration tolerance rather than a
real difference. Second, the cost does depend on it, and the gain from a larger step levels off: a
step a hundred times the default saves two iterations, while a step a hundred times smaller costs
four times as many. Too small a step over-damps the iteration without improving robustness.

The default of $5\times10^{-4}$ s sits at the knee of that curve and gives about the same initial
damping as the block-wide resistance scaled by the cross-momentum coefficients that preceded it.

## Where the step matters more

!! Intentional comment to provide extra spacing

This case needs no retries at any step size, so its damping decays monotonically and the step only
buys iteration count. Flow blockage cases are different: the THORS FFM-5B high-flow case is
sensitive to $\Delta\tau_0$ and reaches the outer-iteration limit for some values between
$2\times10^{-4}$ and $2\times10^{-3}$ s, while the THORS FFM-3A and FFM-5B cases retry their coupled
solves several times per outer iteration at the default. For those cases the guidance in
[Troubleshooting failed solves](user_notes.md#troubleshooting) applies: try values a factor of two
to five either side of the default rather than a single value.
