# MoveNodesByParsedExpressionModifier

!syntax description /UserObjects/MoveNodesByParsedExpressionModifier

Displaces the nodes from the supplied [!param](/UserObjects/MoveNodesByParsedExpressionModifier/boundary)
or [!param](/UserObjects/MoveNodesByParsedExpressionModifier/block) by the parsed expressions
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/displacement_x),
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/displacement_y), and
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/displacement_z). If neither
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/boundary) nor
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/block) is specified, all nodes in the mesh are
transformed.

The displacement of each node is evaluated relative to that node's original (undisplaced)
position, so executing the modifier repeatedly (e.g. on a regular schedule or after adaptivity)
does not accumulate drift. The expressions may reference the node's original coordinates `x`, `y`,
`z` and the time `t`, as well as nodal variables listed in
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/coupled_variables) (an error is raised for
non-nodal variables), [!param](/UserObjects/MoveNodesByParsedExpressionModifier/functions),
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/postprocessors),
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/functor_names), and constants defined through
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/constant_names) and
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/constant_expressions).
Coordinates and any referenced functions or functors are evaluated at each node's original
(undisplaced) position.

The modifier displaces the mesh actively on its own execution schedule, set by the
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/execute_on) parameter (it does not respond to the
problem's mesh-changed event like [`MoveNodesToSphere`](/MoveNodesToSphere.md) does). Set
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/notify_mesh_changed) to `true` to notify the
problem that the mesh has changed after the nodes are moved, so that mesh-dependent caches, the
displaced mesh, geometric searches, and output reflect the new node positions.

When [!param](/UserObjects/MoveNodesByParsedExpressionModifier/coupled_variables) are used in parallel, the
referenced variable values are gathered onto all ranks each time the modifier runs so that nodal
values can be read at any moved node. This serialized copy uses memory proportional to the number of
degrees of freedom per rank.

## Functors

Any [functor](Functors/index.md), such as a functor material property, can be made available to the
displacement expressions by listing it in
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/functor_names). By default a functor is
referred to in the expressions by its own name; supply
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/functor_symbols) to give each functor a
different symbol instead (one symbol per functor). Functors are evaluated at the node without a
block connection, as elsewhere in MOOSE's nodal functor evaluations, so a functor material property
must be defined on every block of the mesh to be usable here.

Variables cannot be passed through
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/functor_names); use
[!param](/UserObjects/MoveNodesByParsedExpressionModifier/coupled_variables) instead, which gathers the
nodal values needed to displace nodes owned by other processors (see above). An error is raised if a
variable name is given as a functor.

!listing test/tests/meshmodifiers/move_nodes_by_parsed_expression_modifier/functors.i block=UserObjects

## Optional outputs

Three optional outputs can be enabled, each independently and only when its parameter is
supplied. The target auxiliary variables must be created by the user in the input (with the
names below); the modifier writes into them.

- [!param](/UserObjects/MoveNodesByParsedExpressionModifier/original_coordinate_variables): writes the
  original (undisplaced) node coordinates to the three named nodal aux variables, given in
  x, y, z order.
- [!param](/UserObjects/MoveNodesByParsedExpressionModifier/displacement_variables): writes the current
  node displacement (current minus original position) to the three named nodal aux variables,
  given in x, y, z order.
- [!param](/UserObjects/MoveNodesByParsedExpressionModifier/density_factor_variable): writes a
  per-element density adjustment factor to the named elemental (`MONOMIAL`, `CONSTANT`) aux
  variable. The factor is the original element volume divided by the current element volume
  (equivalently `1/det(F)`); multiplying a strain-free density by it conserves mass, analogous
  to the [`StrainAdjustedDensity`](StrainAdjustedDensity.md optional=True) material. Element
  volumes are coordinate-aware (XYZ, RZ, and RSPHERICAL). The original volumes are captured at
  the modifier's first execution, which must precede any displacement of these nodes.

!syntax parameters /UserObjects/MoveNodesByParsedExpressionModifier

!syntax inputs /UserObjects/MoveNodesByParsedExpressionModifier

!syntax children /UserObjects/MoveNodesByParsedExpressionModifier
