# SolutionInvalidity

## Description

The [/SolutionInvalidity.md] object holds solution invalid warning information for MOOSE. With this object, you can mark a solution as "invalid" and output which object and how many times the warning occurs. An invalid solution means that the solution somehow does not satisfy requirements such as a value being out of the bounds of a correlation.  Solutions are allowed to be invalid *during* the nonlinear solve - but are not allowed to be invalid once it converges. A "converged" solution that is marked as invalid will cause MOOSE to behave as if the solution did NOT converge - including cutting back timesteps, etc.

To declare a solution as "invalid", use the macro in the following code to mark it. The user could provide a short description of the object(optional) and a message to describe the invalidity(*required*):

!listing /test/src/materials/NonsafeMaterial.C  re=if \(_input_diffusivity.*\s*.\s*flagInvalidSolution\(.*\s*flagInvalidSolution\(.*\s*};

*It is recommended to have a short description for each object especially when you want to mark multiple types of invalid solutions within one object.*

If any solution invalidity is detected during the solve, a summary table of solution invalid warnings will be generated and reported at the end of each time step as below:

```
Solution Invalid Warnings:
------------------------------------------------------------------------------------------------------------------------------
|                 Object                 | Current | Timestep | Total |                        Message                       |
------------------------------------------------------------------------------------------------------------------------------
| NonsafeMaterial::Diffusivity check     |      16 |       64 |    64 | The diffusivity is greater than the threshold value! |
| NonsafeMaterial::Second thing to check |      16 |       64 |    64 | Extra invalid thing!                                 |
------------------------------------------------------------------------------------------------------------------------------
```

1. *Object*: shows the moose object name registered for the solution invalid detection and an optional discription
2. *Current*: shows the number of solution invalid warnings for the latest iteration
3. *Timestep*: shows the number of solution invalid warnings for one time step
4. *Total*: shows the total number of soluton invalid warnings for the solve
5. *Message*: shows the decription of the solution invalidity

This Solution Invalid Warning table can be silenced by setting [!param](/Problem/FEProblem/allow_invalid_solution) to 'true'. Then the converged solution will be allowed even there are still solution invalid warnings, but a message will be generated in the end of the calculation as a reminder:

```
*** Warning ***
The Solution Invalidity warnings are detected but silenced! Use Problem/allow_invalid_solution=false to activate
```

The Solution Invalid Warning can also be printed out immediately after it is detected by setting [!param](/Problem/FEProblem/immediately_print_invalid_solution) to `true`.


