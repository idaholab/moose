# SolutionInvalidityOutput

!syntax description /Outputs/SolutionInvalidityOutput

## Description

The [/SolutionInvalidity.md] object holds solution invalid warning information for MOOSE. With this object, you can mark a solution as "invalid" and output which section and how many times the warning occurs. An invalid solution means that the solution somehow does not satisfy requirements such as a value being out of the bounds of a correlation.  Solutions are allowed to be invalid _during_ the nonlinear solve - but are not allowed to be invalid once it converges. A "converged" solution that is marked as invalid will cause MOOSE to behave as if the solution did NOT converge - including cutting back timesteps, etc.

To set it up, the section is required to be registered using "registerInvalidSection('section name of your choice')". This function will return a unique ID for the section you want to mark. When the solution doesn't satisfy requirements such as a value being out of the bounds of a correlation, the unique ID can be used to mark the solution "invalid". An Example of marking "invalid" material properties is as below:

```
  static const auto solution_id = registerInvalidSection("NonsafeMaterial::computeQpProperties");
  if (material_property > _threshold)
  {
    setSolutionInvalid(solution_id);
  }
```

If any solution invalidity is detected during the nonlinear solve, a summary table of solution invalidity will be generated and reported at the end of each time step as below:

```
The Summary Table of Solution Invalidity Occurences:
------------------------------------------------
|                Section               | Calls |
------------------------------------------------
| NonsafeMaterial::computeQpProperties |    16 |
------------------------------------------------

```

This can be overriden by setting `Problem/allow_invalid_solution=true`.

!syntax parameters /Outputs/SolutionInvalidityOutput

!syntax inputs /Outputs/SolutionInvalidityOutput

!syntax children /Outputs/SolutionInvalidityOutput

!bibtex bibliography
