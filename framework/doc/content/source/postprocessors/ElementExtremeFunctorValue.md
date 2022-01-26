# ElementExtremeFunctorValue

!syntax description /Postprocessors/ElementExtremeFunctorValue

You can optionally provide a [!param](/Postprocessors/ElementExtremeFunctorValue/proxy_functor),
which will change the behavior of this postprocessor to
find the element at which the proxy functor reaches the max/min value,
and then return the value of the specified functor at that element.

!alert note
Unlike the [ElementExtremeValue.md] postprocessor, this postprocessor does not examine the quadrature
point values, but instead passes the element argument to the functor.

## Example Input File Syntax

!listing test/tests/postprocessors/element_extreme_functor_value/extreme_proxy_value.i block=Postprocessors

!syntax parameters /Postprocessors/ElementExtremeFunctorValue

!syntax inputs /Postprocessors/ElementExtremeFunctorValue

!syntax children /Postprocessors/ElementExtremeFunctorValue
