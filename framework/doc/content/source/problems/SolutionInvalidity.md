# SolutionInvalidity

## Description

The [/SolutionInvalidity.md] object holds solution invalid warning information for MOOSE. This object allows you to mark a solution as "invalid" under certain conditions and output in which objects and how many times the conditions occurs. An invalid solution means that the solution somehow does not satisfy requirements, such as a value being out of the bounds of a correlation.  Solutions are allowed to be invalid *during* the nonlinear solve, but they are not allowed to be invalid once the solve has converged. A "converged" solution that is marked as invalid will cause MOOSE to behave as if the solution did NOT converge - including cutting back timesteps, etc.

To declare a solution as "invalid", use the macro in the following code to mark it. The user can provide a message to describe the reason for invalidity:

!listing /test/src/materials/NonsafeMaterial.C  re=\s+if \(_fe_problem.dt\(\) < 1 && _test_invalid_recover\)(?:.|\n)*\}

The `flagInvalidSolution` macro will mark the solution as not converged when triggered. The `flagSolutionWarning` will not do so but simply report on the number of warnings encountered.

!alert tip
It is recommended to have a unique message for each invalidity, especially when you want to mark multiple types of invalid solutions within one object.

If any solution invalidity is detected during the solve, a summary table of solution invalid warnings will be generated and reported at the end of each time step. An example of this output is shown below:

```
Solution Invalid Warnings:
---------------------------------------------------------------------------------------------------------
|     Object      | Converged | Timestep | Total |                        Message                       |
---------------------------------------------------------------------------------------------------------
| NonsafeMaterial |        16 |       48 |    48 | The diffusivity is greater than the threshold value! |
| NonsafeMaterial |        16 |       48 |    48 | Extra invalid thing!                                 |
---------------------------------------------------------------------------------------------------------
```

1. *Object*: shows the moose object name registered for the solution invalid detection and an optional description
2. *Converged*: shows the number of solution invalid warnings for the latest iteration, at convergence
3. *Timestep*: shows the number of solution invalid warnings for the latest time step over all iterations
4. *Total*: shows the total number of solution invalid warnings for the entire simulation
5. *Message*: shows the description of the solution invalidity

When the solution is converged, the number of solution invalid warnings is reported for the latest iteration (linear/nonlinear), the last time step (over all solver iterations) and the entire simulation.
SolutionInvalidity warnings can come from numerous objects, some executed once and some executed on every quadrature point. The number of occurrences reported reflects this.

This Solution Invalid Warnings table can be silenced by setting [!param](/Problem/FEProblem/allow_invalid_solution) to 'true'. Then the converged solution will be accepted even if there are still solution invalid warnings, but a message will be generated at the end of the calculation as a reminder:

```
*** Warning ***
The Solution Invalidity warnings are detected but silenced! Use Problem/show_invalid_solution_console=true to show invalid solution counts
```

The Solution Invalid Warning can also be printed out immediately after it is detected by setting [!param](/Problem/FEProblem/immediately_print_invalid_solution) to `true`.


