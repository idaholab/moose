# ParsedPostprocessor

!syntax description /Postprocessors/ParsedPostprocessor

The [!param](/Postprocessors/ParsedPostprocessor/expression) to parse may only include other postprocessors, the time variable and
constants from the input file.
The expression may use post-processor names directly, or the user may specify [!param](/Postprocessors/ParsedPostprocessor/pp_symbols) to associate a symbol for each of the post-processors in 
[!param](/Postprocessors/ParsedPostprocessor/pp_names) to use in the expression.

!alert note
Derivatives and integrals are not natively supported by the parsing operation, unless the
postprocessors in the parsed expression are already the derivatives / integrals of interest.

## Example Input File Syntax

In this test input file, we compute various quantities using ParsedPostprocessors. First,
we look at the ratio of two postprocessors:

!listing test/tests/postprocessors/parsed_postprocessor/parsed_pp.i block=Postprocessors/parsed

Then we add a time dependence to the postprocessor result:

!listing test/tests/postprocessors/parsed_postprocessor/parsed_pp.i block=Postprocessors/parsed_with_t

And finally we introduce constants from the input file in the parsed expression:

!listing test/tests/postprocessors/parsed_postprocessor/parsed_pp.i block=Postprocessors/parsed_with_constants

!syntax parameters /Postprocessors/ParsedPostprocessor

!syntax inputs /Postprocessors/ParsedPostprocessor

!syntax children /Postprocessors/ParsedPostprocessor
