# ElementsToSimplicesConverter

!syntax description /Mesh/ElementsToSimplicesConverter

An input mesh, as specified in the
[!param](/Mesh/ElementsToSimplicesConverter/input) parameter, will be modified
to replace each non-simplex mesh element with a set of simplices
connecting the same nodes.  Each quad is split into 2 triangles, each
pyramid into 2 tetrahedra, each prism into 3 tets, and each cube into
6 tets.

The input mesh must be "flat", not hierarchically refined.

The algorithm in 2D splits quads along their shortest interior
diagonal, to try to improve element quality.

Currently the algorithm in 3D uses node ids to decide on splitting
directions in a consistent way across face neighbors; this may change
in the future.

In 3D, the geometry of any non-planar quadrilateral faces of cubes
and/or prisms will not be exactly preserved, due to the change from
bilinear or biquadratic mapping on quadrilaterals to linear or
quadratic mapping on the triangles that replace them.

!syntax parameters /Mesh/ElementsToSimplicesConverter

!syntax inputs /Mesh/ElementsToSimplicesConverter

!syntax children /Mesh/ElementsToSimplicesConverter
