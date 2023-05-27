# MaterialFunctorConverter

The `MaterialFunctorConverter` is used to explicitly convert functors, such as [functor material properties](syntax/FunctorMaterials/index.md)
into regular non-AD or AD material properties.

!alert note
If this material is indicated as [!param](/Materials/MaterialFunctorConverter/constant_on) an element or a subdomain,
we always evaluate the functor on the first quadrature point of every element, and we recommend the
[functor caching](syntax/Functors/index.md#caching) be turned on.

!alert note
This `Material` handles the conversion from functor material properties to regular material properties. Unfortunately, the conversion
in the other direction, from regular material properties to functors, is not implemented and would be quite challenging
notably because of the need to handle complex material property dependency resolution.

!syntax parameters /Materials/MaterialFunctorConverter

!syntax inputs /Materials/MaterialFunctorConverter

!syntax children /Materials/MaterialFunctorConverter
