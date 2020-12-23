# RayDistanceAux

!syntax description /RayKernels/RayDistanceAux

The accumulation is achieved by overriding `onSegment` and appending into the `AuxVariable` via `addValue()`, as:

!listing modules/ray_tracing/src/raykernels/RayDistanceAux.C re=void\sRayDistanceAux::onSegment.*?^}

!syntax parameters /RayKernels/RayDistanceAux

!syntax inputs /RayKernels/RayDistanceAux
