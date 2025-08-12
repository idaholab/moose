# NearestPointLayeredSideIntegralFunctor

This object is the same as [NearestPointLayeredSideIntegral.md], but the layered integral may be taken
of any [functor](Functors/index.md), not just a variable.

!alert note title=Functor requirements
This object requires the `ElemSideQpArg` [functor spatial argument](Functors/index.md) to be
implemented for the [!param](/UserObjects/NearestPointLayeredSideIntegralFunctor/functor) parameter.

!alert warning
Note that if [!param](/UserObjects/NearestPointLayeredSideIntegralFunctor/positions_object) is used to specify the nearest points,
only the vector of initial [Positions](syntax/Positions/index.md) are used at this time.
Updates to the 'positions' vector are not supported.

!syntax parameters /UserObjects/NearestPointLayeredSideIntegralFunctor

!syntax inputs /UserObjects/NearestPointLayeredSideIntegralFunctor

!syntax children /UserObjects/NearestPointLayeredSideIntegralFunctor
