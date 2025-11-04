# GenericFunctorTimeDerivativeMaterial

!syntax description /FunctorMaterials/GenericFunctorTimeDerivativeMaterial

## Overview

This object creates [functor material properties](Materials/index.md#functor-props) that are time derivatives of
other functors.

!alert warning
Not all functors can be used as inputs. Many functors, notably off-the-shelf postprocessors do not compute
their time derivative by default. These time derivatives can be derived and implemented as the `dot` functor routine.
Unless this routine is implemented, these functors should not be used as inputs to this functor material.

!alert warning
Time derivatives are not available at all times during the simulation. Notably, on `INITIAL` and `TIMESTEP_BEGIN`,
the time derivatives are a priori NOT available, and any functor material properties created by this object will return 0.

!alert warning
The time derivative routine used by this functor, namely `functor.dot(spatial_argument, state_argument)`, is not implemented for
all spatial arguments and all state arguments for every single functor. Users must be extremely careful in their use
of the functors created by this functor material.

!alert note
All AD-types of the properties defined in this material must match. Variables are automatically
considered as AD functors, even auxiliary variables. The AD version of this material is `ADGenericFunctorTimeDerivativeMaterial`.
Its inputs are a vector of AD functors and it creates AD functor material properties.

!syntax parameters /FunctorMaterials/GenericFunctorTimeDerivativeMaterial

!syntax inputs /FunctorMaterials/GenericFunctorTimeDerivativeMaterial

!syntax children /FunctorMaterials/GenericFunctorTimeDerivativeMaterial
