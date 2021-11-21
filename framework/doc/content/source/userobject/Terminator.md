# Terminator

!syntax description /UserObjects/Terminator

The parsed logical expression is specified with the [!param](). More information about parsed expressions
may be found on the [function parser documentation](http://warp.povusers.org/FunctionParser/)

The `Terminator` can act in two modes, specified by the [!param](/UserObjects/Terminator/fail_mode) :

- HARD failure, the default, will terminate the simulation when the conditions are met

- SOFT failure, will stop the ongoing solve and let the solver try again using a smaller time step, for
  transient simulations.


The message output by the `Terminator` when the condition for termination are met is specified using the
[!param](/UserObjects/Terminator/error_level) parameter. It may be output as:

- an error, forcing a hard failure

- a warning, to raise attention to an issue or abnormal solve conditions

- an information message, to indicate that while the `Terminator` is acting on the solve, the conditions
  met are expected or normal.


## Example input syntax

In this example, the `Terminator` is used to fail a time step solve, based on a criterion
`dt > 20`. Once the solve is soft-failed for this time step, the solver tries again by cutting
the time step. This happens to make the `Terminator` parsed criterion pass, so it does not act again
on this time step.

!listing test/tests/userobjects/Terminator/terminator_soft.i block=UserObjects

!syntax parameters /UserObjects/Terminator

!syntax inputs /UserObjects/Terminator

!syntax children /UserObjects/Terminator
