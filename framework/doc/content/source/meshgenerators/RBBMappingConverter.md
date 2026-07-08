# RBBMappingConverter

!syntax description /Mesh/RBBMappingConverter

An input mesh, as specified in the
[!param](/Mesh/RBBMappingConverter/input) parameter, will be modified
to reinterpolate the points of each mesh element using a
Rational-Bernstein-Bezier mapping rather than a Lagrange mapping from
master to physical element space.  Each node in the mesh becomes a
spline control node, with an associated weight used in rational basis
function calculations, changing the shape of element edges and faces
in between the reinterpolated points.

In cases where the true geometry is exactly representable by a
rational polynomial of the existing element order and the existing
element nodes interpolate that geometry consistently with that
representation (e.g. circles and circular arcs, their extrusions into
cylinders and cylindrical arcs, quad or hex shells with edges that map
latitudinal/longitudinal/radial lines in spherical coordinates) the
resulting remapping will give isogeometric elements, exactly (to
within floating-point error) mapping the true geometry.

Note that, for non-vertex nodes, the position of a spline node may not
match the position of the corresponding physical point which the
spline interpolates.  Most I/O formats such as ExodusII currently
output spline nodes at the control point coordinates, which can
distort visualization.

!syntax parameters /Mesh/RBBMappingConverter

!syntax inputs /Mesh/RBBMappingConverter

!syntax children /Mesh/RBBMappingConverter
