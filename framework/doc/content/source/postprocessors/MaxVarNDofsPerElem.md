# MaxVarNDofsPerElem

## Description

This class can be used to calculate the maximum number of degrees of freedom on
an element. This may be useful in automatic differentiation calculations to
limit the number of derivative calculations that have to be carried out.

!alert note
This postprocessor is a MOOSE test object. Pass `--allow-test-objects` to your MOOSE or
MOOSE-based app executable to be able to use it in a simulation. If this object is needed often, 
it is recommended that the developer [reach out to the MOOSE development team](https://github.com/idaholab/moose/discussions) 
so that the object can be moved from `test/src` or create a new Pull Request (with contribution 
guidelines found [here](https://mooseframework.inl.gov/framework/contributing.html))
containing the move.
