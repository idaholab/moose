# ElementL2FunctorError

The ElementL2FunctorError computes the Euclidean distance between an automatic
differentiation (AD) functor representing an approximate solution and an AD
functor representing an analytical exact solution. This Postprocessor
is very useful for verifying the proper function of the framework through the
[Method of Manufactured Solutions](python/mms.md optional=true). This class is a
generalization to the functor system of [ElementL2Error.md].

!syntax parameters /Postprocessors/ElementL2FunctorError

!syntax inputs /Postprocessors/ElementL2FunctorError

!syntax children /Postprocessors/ElementL2FunctorError
