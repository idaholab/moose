# OptimizationInfo

!syntax description /Reporters/OptimizationInfo

## Overview

`OptimizationInfo` provides the ability to output optimization solve information from [Optimize.md] for each [Tao iteration](https://petsc.org/release/docs/manualpages/Tao/TaoGetSolutionStatus/). The information outputted can be specified with [!param](/Reporters/OptimizationInfo/items). If this parameter is left unspecified, all information is outputted. The values that are optionally declared are shown in [tab:opt_info].

!table id=tab:opt_info caption=List of values declared
| Description | Name | [!param](/Reporters/OptimizationInfo/items) |
| :- | - | - |
| Objective Function Value | `function_value` | `function_value` |
| Gradient Norm | `gnorm` | `gnorm` |
| Infeasibility Norm | `cnorm` | `cnorm` |
| Step Length | `xdiff` | `xdiff` |
| Total Model Evaluations | `current_iterate` | `current_iterate` |
| Forward Model Evaluations | `objective_iterate` | `current_iterate` |
| Adjoint/Gradient Model Evaluations | `gradient_iterate` | `current_iterate` |
| Homogeneous Model Evaluations | `hessian_iterate` | `current_iterate` |
| Total Tao Iterations | `function_solves` | `current_iterate` |

!syntax parameters /Reporters/OptimizationInfo

!syntax inputs /Reporters/OptimizationInfo

!syntax children /Reporters/OptimizationInfo
