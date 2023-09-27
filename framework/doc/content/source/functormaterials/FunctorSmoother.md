# FunctorSmoother

!syntax description /FunctorMaterials/FunctorSmoother

Three heuristics have been implemented for smoothing a functor (or several). These heuristics have
only been implemented for the `ElemArg` functor argument. Functor arguments are explained in
the [Functor documentation](syntax/Functors/index.md).

- face averaging

The elemental value of the smoothed functor is equal to the arithmetic average of the values of the previous
functor on the faces of the element.

- neighbor averaging

The elemental value of the smoothed functor is equal to the arithmetic average of the values of the previous
functor at the neighbors of the element.

- checkerboard removing

The elemental value of the smoothed functor is equal to the average of the current functor element value and the two neighbor values
that are the furthest from the current functor element value.
The logic behind this heuristic is to average neighboring checkerboard values.

The value of the smoothed functor on elements can be visualized using a [FunctorElementalAux.md].

!alert note
No volume or area weighting is currently implemented in the `FunctorSmoother`. Feel free to experiment
with weighting schemes and make a contribution to MOOSE if they are valuable.

!syntax parameters /FunctorMaterials/FunctorSmoother

!syntax inputs /FunctorMaterials/FunctorSmoother

!syntax children /FunctorMaterials/FunctorSmoother
