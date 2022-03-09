# MaxVarNDofsPerElem

## Description

This class can be used to calculate the maximum number of degrees of freedom on
an element. This may be useful in automatic differentiation calculations to
limit the number of derivative calculations that have to be carried out.

!alert note
This postprocessor is a MOOSE test object. Pass `--allow-test-objects` to a `MooseTestApp`
to be able to use it, or migrate it from the MOOSE `test/src` directory to your source directory.
