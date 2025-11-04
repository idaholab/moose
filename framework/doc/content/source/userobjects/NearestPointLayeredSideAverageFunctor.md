# NearestPointLayeredSideAverageFunctor

This object is the same as [NearestPointLayeredSideAverage.md], but the layered average may be taken
of any [functor](Functors/index.md), not just a variable.

!alert note title=Functor requirements
This object requires the `ElemSideQpArg` [functor spatial argument](Functors/index.md) to be
implemented for the [!param](/UserObjects/NearestPointLayeredSideAverageFunctor/functor) parameter.

!alert warning
Note that if [!param](/UserObjects/NearestPointLayeredSideAverageFunctor/positions_object) is used to specify the nearest points,
only the vector of initial [Positions](syntax/Positions/index.md) are used at this time.
Updates to the 'positions' vector are not supported.

!syntax parameters /UserObjects/NearestPointLayeredSideAverageFunctor

!syntax inputs /UserObjects/NearestPointLayeredSideAverageFunctor

!syntax children /UserObjects/NearestPointLayeredSideAverageFunctor
