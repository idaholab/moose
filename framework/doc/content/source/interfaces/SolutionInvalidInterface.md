# SolutionInvalidInterface

## Tracking warning occurences

The `SolutionInvalidInterface` tracks the occurences of `mooseWarning` and `paramWarning` across all [MooseObject.md] and [Action.md].
If any, these occurences are reported by the [SolutionInvalidityOutput.md], displayed at the end of the simulation.

!alert note
To avoid tracking the occurences of a particular warning, the developer may use `MooseBase::mooseWarning` instead of `mooseWarning`. This
explicitly calls the `MooseBase` implementation of `mooseWarning` instead of the implementation in the `SolutionInvalidInterface`.

## Declaring a solution as invalid

The `SolutionInvalidInterface` defines the method used to mark a solution as "invalid".  An invalid solution means that the solution somehow does not satisfy requirements such as a value being out of bounds of a correlation.  Solutions are allowed to be invalid _during_ the nonlinear solve - but are not allowed to invalid once it converges.  A "converged" solution that is marked as invalid will cause MOOSE to behave as if the solution did NOT converge - including cutting back timesteps, etc.

This can be overridden by setting `Problem/allow_invalid_solution=true`.

!listing /SolutionInvalidInterface.h start=doco-normal-methods-begin end=doco-normal-methods-end include-start=false

