# FunctorADConverter

!syntax description /FunctorMaterials/FunctorADConverter

Converting from AD to regular functors or vice versa can both lead to irremediable loss of
derivative information when using the [automatic differentiation system](systems/NonlinearSystem.md#AD).
Missing derivative information (from using a converted regular functor where a true AD functor should have been used)
can lead to an imperfect Jacobian which can impact convergence properties of Newton solves.

Some examples of safe conversions:

- Functions to ADFunctions and vice-versa are safe since functions do not hold derivative data

- AuxVariables to regular functors is safe since auxiliary variables do not hold derivative data


## Example input syntax

An example of some gymnastics with functor conversions is shown in this example. The reader should note
that only the conversion to an AD functor from a regular functor (from a regular Function) was necessary.

!listing test/tests/materials/functor_properties/ad_conversion/1d_dirichlet.i block=Materials

!syntax parameters /FunctorMaterials/FunctorADConverter

!syntax inputs /FunctorMaterials/FunctorADConverter

!syntax children /FunctorMaterials/FunctorADConverter
