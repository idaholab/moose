# (AD)FunctorChangeFunctorMaterial

This [functor material](FunctorMaterials/index.md) adds a functor material property
that computes the change in a [functor](/Functors/index.md) over one of the following:

- time step
- nonlinear iteration
- [MultiApp fixed point iteration](FixedPointAlgorithms/index.md)

!alert tip title=Useful for step convergence criteria
One use of this functor material is as a step criteria in assessing convergence for
various solves using the [Convergence system](Convergence/index.md), in conjunction
with a post-processor such as [ElementExtremeFunctorValue.md].

!syntax parameters /FunctorMaterials/FunctorChangeFunctorMaterial

!syntax inputs /FunctorMaterials/FunctorChangeFunctorMaterial

!syntax children /FunctorMaterials/FunctorChangeFunctorMaterial
