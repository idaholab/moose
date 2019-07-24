# PostprocessorInterface

The PostprocessorInterface defines the methods used for retrieving PostprocessorValue references. Many objects
in MOOSE support the retrieval of these references for use in calculations. Postprocessors being "post"-processors
generally execute after most other systems in MOOSE so these values are often lagged when being used in another
calculation.

!listing /PostprocessorInterface.h start=doco-normal-methods-begin end=doco-normal-methods-end include-start=false
