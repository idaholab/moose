# A QUAD4 with a colinear vertex (1,1) that is ALSO a genuine corner of a second quad sharing it.
# Because the vertex is not redundant in every element that uses it, dropping it would leave a
# hanging node, so the repair conservatively leaves the quad in place and reports it.
[Mesh]
  [quad_colinear]
    type = ElementGenerator
    nodal_positions = '0 0 0   2 0 0   2 2 0   1 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [quad_corner]
    type = ElementGenerator
    input = quad_colinear
    nodal_positions = '1 1 0   1 -1 0   3 -1 0   3 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [repair]
    type = MeshRepairGenerator
    input = quad_corner
    fix_node_overlap = true
    fix_degenerate_elements = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
