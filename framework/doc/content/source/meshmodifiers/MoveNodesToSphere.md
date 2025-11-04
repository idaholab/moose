# MoveNodesToSphere

!syntax description /UserObjects/MoveNodesToSphere

Moves all nodes from the supplied [!param](/UserObjects/MoveNodesToSphere/boundary) or [!param](/UserObjects/MoveNodesToSphere/block)
to the surface of the sphere specified by the [!param](/UserObjects/MoveNodesToSphere/center) and
[!param](/UserObjects/MoveNodesToSphere/radius) parameters.

This is performed by default during adaptivity to allow adaptive refinement of curved geometries.
It can also be performed using regular execution schedules specified by the [!param](/UserObjects/MoveNodesToSphere/execute_on)
parameter.

!syntax parameters /UserObjects/MoveNodesToSphere

!syntax inputs /UserObjects/MoveNodesToSphere

!syntax children /UserObjects/MoveNodesToSphere
