# ParsedVectorAux

!syntax description /AuxKernels/ParsedVectorAux

The parsed expressions, `expression_x`/`y`/`z` for each component of the vector, may contain:

- variables ([!param](/AuxKernels/ParsedVectorAux/coupled_variables) parameter)

- vector variables ([!param](/AuxKernels/ParsedVectorAux/coupled_vector_variables) parameter)

- coordinates in space and time ([!param](/AuxKernels/ParsedVectorAux/use_xyzt) parameter then add to `expression_x`/`y`/`z`)

- constants ([!param](/AuxKernels/ParsedVectorAux/constant_names) for their name in the expression and [!param](/AuxKernels/ParsedVectorAux/constant_expressions) for their values)


Material properties are currently not supported, but it would be really easy to add it so feel free to contact us.

!syntax parameters /AuxKernels/ParsedVectorAux

!syntax inputs /AuxKernels/ParsedVectorAux

!syntax children /AuxKernels/ParsedVectorAux
