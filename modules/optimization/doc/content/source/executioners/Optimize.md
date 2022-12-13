# Optimize

!syntax description /Executioner/Optimize

## Overview

This is executioner performs optimization using the [TAO](https://petsc.org/release/docs/manual/tao/) optimization toolkit. The specific algorithm used is specified using the [!param](/Executioner/Optimize/tao_solver) parameter. The options and connection with [TAO algorithms](https://petsc.org/release/docs/manual/tao/#sec-tao-solvers) is shown in [tab:tao_solver]. Additional solver options can be given to the TAO solver using the [!param](/Executioner/Optimize/petsc_options), [!param](/Executioner/Optimize/petsc_options_iname), and [!param](/Executioner/Optimize/petsc_options_value) parameters or from command-line.

!table id=tab:tao_solver caption=List of available optimization algorithms
| Algorithm | [!param](/Executioner/Optimize/tao_solver) | TAO Option | `tao_type` |
| - | - | - | - |
| Newton Trust Region | `taontr` | [TAONTR](https://petsc.org/release/docs/manualpages/Tao/TAONTR/) | `ntr` |
| Bounded Newton Trust Region | `taobntr` | [TAOBNTR](https://petsc.org/release/docs/manualpages/Tao/TAOBNTR/) | `bntr` |
| Bounded Conjugate Gradient | `taobncg` | [TAOBNCG](https://petsc.org/release/docs/manualpages/Tao/TAOBNCG/) | `bncg` |
| Newton Line Search | `taonls` | [TAONLS](https://petsc.org/release/docs/manualpages/Tao/TAONLS/) | `nls` |
| Bounded Newton Line Search | `taobnls` | [TAOBNLS](https://petsc.org/release/docs/manualpages/Tao/TAOBNLS/) | `bnls` |
| Limited Memory Variable Metric | `taolmvm` | [TAOLMVM](https://petsc.org/release/docs/manualpages/Tao/TAOLMVM/) | `lmvm` |
| Bounded Limited Memory Variable Metric | `taoblmvm` | [TAOBLMVM](https://petsc.org/release/docs/manualpages/Tao/TAOBLMVM/) | `blmvm` |
| Nelder-Mead | `taonm` | [TAONM](https://petsc.org/release/docs/manualpages/Tao/TAONM/) | `nm` |
| Bounded Quasi-Newton Line Search | `taobqnls` | [TAOBQNLS](https://petsc.org/release/docs/manualpages/Tao/TAOBQNLS/) | `bqnls` |
| Orthant-wise Limited Memory | `taoowlqn` | [TAOOWLQN](https://petsc.org/release/docs/manualpages/Tao/TAOOWLQN/) | `taoowlqn` |
| Gradient Projection Conjugate Gradient | `taogpcg` | [TAOGPCG](https://petsc.org/release/docs/manualpages/Tao/TAOGPCG/) | `gpcg` |
| Bundle Method for Regularized Risk Minimization | `taobmrm` | [TAOBMRM](https://petsc.org/release/docs/manualpages/Tao/TAOBMRM/) | `bmrm` |

This executioner relies on a [OptimizationReporter](OptimizationReporter/index.md) to define the bounds, objective, and gradient of the form function. The objective is defined by the `computeObjective` member in the [OptimizationReporter](OptimizationReporter.h) class. the gradient is defined by `computeGradient` and the bounds are defined by `getUpperBounds` and `getLowerBounds`. Whether it is necessary to define each of these members is based on whether the selected algorithm needs it, see [Summary of Tao Solvers](https://petsc.org/release/overview/tao_solve_table/) for more information. The Hessian is computed using a matrix-free method, where it evaluates the action of the Hessian on the form function parameters (the values that are being optimized). It does this by evaluating a homogeneous version of the objective function and subsequently computes the gradient.

To aid in the computation of the objective, gradient, and Hessian, this execuationer includes additional execution flags that MOOSE objects (like [MultiApps](MultiApps/index.md)) can be evaluated on. Having `execute_on = forward` will execute the object(s) just before `computeObjective` is called and `execute_on = adjoint` will execute the object()s just before `computeGradient` is called.  Having `execute_on = homogeneous_forward` will execute the object(s) during the matrix-free Hessian computation, before calling `adjoint` and `computeGradient`.

The form function's parameters are represented as a vector of values and is tied to reporter values within the [OptimizationReporter](OptimizationReporter/index.md).

## Example Input Syntax

The following performs optimization using th Newton line search algorithm. The additional options set the optimization tolerance to $10^{-5}$, set the max iterations to 10, and set the linear solver type to conjugate gradient with no preconditioning.

!listing test/tests/optimizationreporter/point_loads/main.i
         block=Executioner

!syntax parameters /Executioner/Optimize

!syntax inputs /Executioner/Optimize

!syntax children /Executioner/Optimize
