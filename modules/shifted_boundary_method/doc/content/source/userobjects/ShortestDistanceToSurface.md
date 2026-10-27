# ShortestDistanceToSurface

`ShortestDistanceToSurface` is an `ElementUserObject` that evaluates the minimum
distance to one or more implicit surfaces defined by MOOSE functions. It accepts both
analytical level sets (via [`ParsedFunction`](syntax/Functions/index.md)) and
mesh-based boundaries (via
[`UnsignedDistanceToSurfaceMesh`](functions/UnsignedDistanceToSurfaceMesh.md)),
aggregating them to identify the closest surface for any given element.

!syntax description /UserObjects/ShortestDistanceToSurface

!syntax parameters /UserObjects/ShortestDistanceToSurface

!syntax inputs /UserObjects/ShortestDistanceToSurface

!syntax children /UserObjects/ShortestDistanceToSurface
