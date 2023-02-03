# SolutionInvalidity

## Description

The [/SolutionInvalidity.md] object holds solution invalid warning information for MOOSE. With this object, you can mark a solution as "invalid" and output which object and how many times the warning occurs. An invalid solution means that the solution somehow does not satisfy requirements, such as a value being out of the bounds of a correlation.  Solutions are allowed to be invalid *during* the nonlinear solve, but they are not allowed to be invalid once it converges. A "converged" solution that is marked as invalid will cause MOOSE to behave as if the solution did NOT converge - including cutting back timesteps, etc.

To declare a solution as "invalid", use the macro in the following code to mark it. The user can provide a message to describe the invalidity:

!listing /test/src/materials/NonsafeMaterial.C  re=\s+if \(_input_diffusivity > _threshold\).*?\}

!alert tip
It is recommended to have a unique message for each invalidity, especially when you want to mark multiple types of invalid solutions within one object.

If any solution invalidity is detected during the solve, a summary table of solution invalid warnings will be generated and reported at the end of each time step. An example of this output is shown below:

```
Solution Invalid Warnings:
------------------------------------------------------------------------------------------------------
|     Object      | Latest | Timestep | Total |                        Message                       |
------------------------------------------------------------------------------------------------------
| NonsafeMaterial |     16 |       64 |    64 | The diffusivity is greater than the threshold value! |
| NonsafeMaterial |     16 |       64 |    64 | Extra invalid thing!                                 |
------------------------------------------------------------------------------------------------------
```

1. *Object*: shows the moose object name registered for the solution invalid detection and an optional description
2. *Latest*: shows the number of solution invalid warnings for the latest iteration
3. *Timestep*: shows the number of solution invalid warnings for the latest time step
4. *Total*: shows the total number of solution invalid warnings for the entire simulation
5. *Message*: shows the description of the solution invalidity

SolutionInvalidity will check the value of the quadrature points during each linear and nonlinear iterations. When the solution is converged, the number of solution invalid warnings is reported for the latest iteration(linear/nonlinear), the last time step and the entire simulation.

This Solution Invalid Warnings table can be silenced by setting [!param](/Problem/FEProblem/allow_invalid_solution) to 'true'. Then the converged solution will be accepted even if there are still solution invalid warnings, but a message will be generated in the end of the calculation as a reminder:

```
*** Warning ***
The Solution Invalidity warnings are detected but silenced! Use Problem/allow_invalid_solution=false to activate
```

The Solution Invalid Warning can also be printed out immediately after it is detected by setting [!param](/Problem/FEProblem/immediately_print_invalid_solution) to `true`.


