# RBBMappingConverter

!syntax description /Mesh/RBBMappingConverter

An input mesh, as specified in the
[!param](/Mesh/RBBMappingConverter/input) parameter, will be modified
to reinterpolate the nodes of each mesh element using a
Rational-Bernstein-Bezier mapping rather than a Lagrange mapping from
master to physical element space.  Each node in the mesh becomes a
spline control node, which for non-vertex nodes may not match the
position of the corresponding physical node.

In cases where the true geometry is exactly representable by a
rational polynomial of the existing element order and the existing
element nodes interpolate that geometry consistently with that
representation (e.g. circles and circular arcs, their extrusions into
cylinders and cylindrical arcs, quad or hex shells with edges that map
latitudinal/longitudinal/radial lines in spherical coordinates) the
resulting remapping will give isogeometric elements, exactly (to
within floating-point error) mapping the true geometry.

!syntax parameters /Mesh/RBBMappingConverter

!syntax inputs /Mesh/RBBMappingConverter

!syntax children /Mesh/RBBMappingConverter
