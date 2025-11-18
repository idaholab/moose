# SBMBndTri3

`SBMBndTri3` provides the interface for handling 3-node triangle elements (`TRI3`) within
the Shifted Boundary Method (SBM) framework. It specializes the generic boundary
element class to perform geometric operations specifically in a three-dimensional
context.

Internally, the class manages the computation of unit outward normals, bounding spheres
for spatial indexing, and robust ray–triangle intersection tests. These capabilities
allow the SBM system to correctly treat 2D surface meshes embedded in 3D problem
domains, enabling consistent distance and intersection queries.

!syntax description /SBMBndElement/SBMBndTri3

!syntax parameters /SBMBndElement/SBMBndTri3

!syntax inputs /SBMBndElement/SBMBndTri3

!syntax children /SBMBndElement/SBMBndTri3
