# ParsedGradFunction

!syntax description /Functions/ParsedGradFunction

The `ParsedGradFunction` is similar to the [MooseParsedFunction.md], differing only in that
it also defines the gradient of the function. Inputs for defining each component of
the gradient follow the same rules as outlined in [MooseParsedFunction.md].

## Example input syntax

In this input, the `ParsedGradFunction` is used for using the
[Method of Manufactured Solutions](python/mms.md optional=true). This method verifies the
convergence of the finite element method to known analytical solutions of a simple problem.
`u_func` is used in the `PostProcessors` block to compute the H1 error, or the error on both
the solution and its gradient, between `u` and this known solution.

!listing test/tests/postprocessors/mms_slope/mms_slope_test.i block=Functions

!syntax parameters /Functions/ParsedGradFunction

!syntax inputs /Functions/ParsedGradFunction

!syntax children /Functions/ParsedGradFunction
