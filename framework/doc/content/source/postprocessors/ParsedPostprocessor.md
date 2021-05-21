# ParsedPostprocessor

!syntax description /Postprocessors/ParsedPostprocessor

The `function` to parse may only include other postprocessors, the time variable and
constants from the input file.

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
