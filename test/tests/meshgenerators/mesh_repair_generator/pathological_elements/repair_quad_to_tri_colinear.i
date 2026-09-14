# A QUAD4 whose fourth vertex (1,1) lies on the diagonal between its neighbors (0,0) and (2,2) - a
# colinear "not sticking out" vertex, with full-length edges. It is redundant only in this element
# (its two edges are shared with nothing else), so removing it (QUAD4 -> TRI3) is conformal. A
# healthy quad shares the full edge (2,0)-(2,2) and is untouched.
[Mesh]
  [quad_colinear]
    type = ElementGenerator
    nodal_positions = '0 0 0   2 0 0   2 2 0   1 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [quad_healthy]
    type = ElementGenerator
    input = quad_colinear
    nodal_positions = '2 0 0   4 0 0   4 2 0   2 2 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [repair]
    type = MeshRepairGenerator
    input = quad_healthy
    fix_node_overlap = true
    fix_degenerate_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_element_volumes = ERROR
    examine_element_overlap = ERROR
    examine_non_conformality = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
