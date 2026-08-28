# MoveNodesByParsedExpression

!syntax description /UserObjects/MoveNodesByParsedExpression

Displaces the nodes from the supplied [!param](/UserObjects/MoveNodesByParsedExpression/boundary)
or [!param](/UserObjects/MoveNodesByParsedExpression/block) by the parsed expressions
[!param](/UserObjects/MoveNodesByParsedExpression/displacement_x),
[!param](/UserObjects/MoveNodesByParsedExpression/displacement_y), and
[!param](/UserObjects/MoveNodesByParsedExpression/displacement_z). If neither
[!param](/UserObjects/MoveNodesByParsedExpression/boundary) nor
[!param](/UserObjects/MoveNodesByParsedExpression/block) is specified, all nodes in the mesh are
transformed.

The displacement of each node is evaluated relative to that node's original (undisplaced)
position, so executing the modifier repeatedly (e.g. on a regular schedule or after adaptivity)
does not accumulate drift. The expressions may reference the node's original coordinates `x`, `y`,
`z` and the time `t`, as well as nodal variables listed in
[!param](/UserObjects/MoveNodesByParsedExpression/coupled_variables) (an error is raised for
non-nodal variables), [!param](/UserObjects/MoveNodesByParsedExpression/functions),
[!param](/UserObjects/MoveNodesByParsedExpression/postprocessors), and constants defined through
[!param](/UserObjects/MoveNodesByParsedExpression/constant_names) and
[!param](/UserObjects/MoveNodesByParsedExpression/constant_expressions).
Coordinates and any referenced functions are evaluated at each node's original (undisplaced) position.

The modifier displaces the mesh actively on its own execution schedule, set by the
[!param](/UserObjects/MoveNodesByParsedExpression/execute_on) parameter (it does not respond to the
problem's mesh-changed event like [`MoveNodesToSphere`](/MoveNodesToSphere.md) does). Set
[!param](/UserObjects/MoveNodesByParsedExpression/notify_mesh_changed) to `true` to notify the
problem that the mesh has changed after the nodes are moved, so that mesh-dependent caches, the
displaced mesh, geometric searches, and output reflect the new node positions.

When [!param](/UserObjects/MoveNodesByParsedExpression/coupled_variables) are used in parallel, the
referenced variable values are gathered onto all ranks each time the modifier runs so that nodal
values can be read at any moved node. This serialized copy uses memory proportional to the number of
degrees of freedom per rank.

## Optional outputs

Three optional outputs can be enabled, each independently and only when its parameter is
supplied. The target auxiliary variables must be created by the user in the input (with the
names below); the modifier writes into them.

- [!param](/UserObjects/MoveNodesByParsedExpression/original_coordinate_variables): writes the
  original (undisplaced) node coordinates to the three named nodal aux variables, given in
  x, y, z order.
- [!param](/UserObjects/MoveNodesByParsedExpression/displacement_variables): writes the current
  node displacement (current minus original position) to the three named nodal aux variables,
  given in x, y, z order.
- [!param](/UserObjects/MoveNodesByParsedExpression/density_factor_variable): writes a
  per-element density adjustment factor to the named elemental (`MONOMIAL`, `CONSTANT`) aux
  variable. The factor is the original element volume divided by the current element volume
  (equivalently `1/det(F)`); multiplying a strain-free density by it conserves mass, analogous
  to the `StrainAdjustedDensity` material. Element volumes are coordinate-aware (XYZ, RZ, and
  RSPHERICAL). The original volumes are captured at the modifier's first execution, which must
  precede any displacement of these nodes.

!syntax parameters /UserObjects/MoveNodesByParsedExpression

!syntax inputs /UserObjects/MoveNodesByParsedExpression

!syntax children /UserObjects/MoveNodesByParsedExpression
