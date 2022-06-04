# FunctionValuePostprocessor

!syntax description /Postprocessors/FunctionValuePostprocessor

If `FunctionValuePostprocessor` uses something like a [/MooseParsedFunction.md],
it may have indirect dependencies on other user objects/postprocessors, since
`ParsedFunction` supports postprocessor values. If this is the case, the
[!param](/Postprocessors/FunctionValuePostprocessor/indirect_dependencies)
parameter should be used to supply these indirect dependencies, otherwise these
dependencies may execute after this postprocessor, and this postprocessor may
have inaccurate values.

!syntax parameters /Postprocessors/FunctionValuePostprocessor

!syntax inputs /Postprocessors/FunctionValuePostprocessor

!syntax children /Postprocessors/FunctionValuePostprocessor
