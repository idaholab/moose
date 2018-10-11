# ADMaterials System

An `ADMaterial` is a class template that an application developer should inherit
from if they want to define material properties whose dependence on coupled
variables should be computed automatically via automatic differentiation. Any
material derived from `ADMaterial` should itself be a class template. This
template will then be used to create two objects: an `ADMaterial<RESIDUAL>` and
a `ADMaterial<JACOBIAN>`. The former object will be called upon during residual
calculations and will not compute any derivatives with respect to the coupled
variables; the latter will be called during Jacobian calculations and *will* do
derivative calculations. When coupling in the non-linear variable, the
application developer should call on the `adCoupled` set of methods defined in
the `Coupleable` class.

!syntax list /ADMaterials objects=True actions=False subsystems=False

!syntax list /ADMaterials objects=False actions=False subsystems=True

!syntax list /ADMaterials objects=False actions=True subsystems=False
