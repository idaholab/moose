# ReflectRayBC

!syntax description /RayBCs/ReflectRayBC

It can properly handle reflections at domain corners/edges that are reflecting on all boundaries at the corner/edge (see [syntax/RayBCs/index.md#hitting-multiple-boundaries] for more information).

It achieves the reflection by changing the direction of the [Ray.md]:

!listing modules/ray_tracing/src/raybcs/ReflectRayBC.C re=void\sReflectRayBC::onBoundary.*?^}

Per the specularly reflected direction (static and available for other [RayBCs/index.md] to use):

!listing modules/ray_tracing/src/raybcs/ReflectRayBC.C re=Point\sReflectRayBC::reflectedDirection.*?^}

!syntax parameters /RayBCs/ReflectRayBC

!syntax inputs /RayBCs/ReflectRayBC
