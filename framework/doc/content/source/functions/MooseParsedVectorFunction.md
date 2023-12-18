# ParsedVectorFunction

!syntax description /Functions/ParsedVectorFunction

The `ParsedVectorFunction` is a vectorized version of the [MooseParsedFunction.md].
The vector components, the curl and the divergence of the function may be specified.
The inputs for the components of the vector, the curl and the divergence of the
function follow the same rules as outlined for the [MooseParsedFunction.md].

## Example input syntax

In this example, a `ParsedVectorFunction` is used to define a curl boundary condition.
Both x,y component and the z-curl are set for this function.

!listing functions/parsed/function_curl.i block=Functions BCs

!syntax parameters /Functions/ParsedVectorFunction

!syntax inputs /Functions/ParsedVectorFunction

!syntax children /Functions/ParsedVectorFunction
