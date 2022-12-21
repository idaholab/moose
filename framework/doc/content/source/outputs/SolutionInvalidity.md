# SolutionInvalidity

## Description

The [/SolutionInvalidity.md] object holds solution invalid warning information for MOOSE. With this object, you can mark a solution as "invalid" and output which section and how many times the warning occurs. An invalid solution means that the solution somehow does not satisfy requirements such as a value being out of the bounds of a correlation.  Solutions are allowed to be invalid _during_ the nonlinear solve - but are not allowed to be invalid once it converges. A "converged" solution that is marked as invalid will cause MOOSE to behave as if the solution did NOT converge - including cutting back timesteps, etc.

To set it up, the section is required to be registered using "registerInvalidSection('section name of your choice')" and an optional message to describe the solution invalidity. This function will return a unique ID for the section you want to mark. When the solution doesn't satisfy requirements such as a value being out of the bounds of a correlation, Then you can mark the solution "invalid" with section IDs. An Example of marking "invalid" material properties is as below:

```
  static const auto solution_id = registerInvalidSection("NonsafeMaterial");
  if (material_property > _threshold)
  {
    setSolutionInvalid(solution_id);
  }
```

If any solution invalidity is detected during the nonlinear solve, a summary table of solution invalid warnings will be generated and reported at the end of each time step as below:

```
Solution Invalid Warnings:
-------------------------------------------------------------------------------------------------------
|     Section     | Current | Timestep | Total |                        Message                       |
-------------------------------------------------------------------------------------------------------
| NonsafeMaterial |      16 |       64 |    64 | The diffusivity is greater than the threshold value! |
-------------------------------------------------------------------------------------------------------

```
**section** shows the section name registered for the solution invalid detection.
**Current** shows the number of solution invalid warnings for the latest iteration.
**Timestep** shows the number of solution invalid warnings for one time step.
**Total** shows the total number of soluton invalid warnings for the calculation.
**Message** shows the decription of the solution invalidity(optional).

This Solution Invalid Warning table can be silenced by setting `Problem/allow_invalid_solution=true`. Then the converged solution will be allowed even there are still solution invalid warnings, but a message will be generated in the end of the calculation as a reminder:
```
*** Warning ***
The Solution Invalidity warnings are detected but silenced! Use Problem/allow_invalid_solution=false to activate

```

The Solution Invalid Warning can also be printed out immediately after it is detected by setting `Problem/immediately_print_invalid_solution=true`.

