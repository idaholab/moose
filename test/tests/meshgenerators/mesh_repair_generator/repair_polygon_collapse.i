# Two hexagonal C0POLYGONs sharing a vertical edge that carries a colinear midpoint vertex (1,1) on
# the segment from (1,0) to (1,2). The vertex is redundant in both polygons and both are polygons,
# so collapsing it reduces each hexagon to a pentagon (n -> n-1) while keeping the mesh conformal.
[Mesh]
  [poly_left]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   1 1 0   1 2 0   0 2 0   -1 1 0'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = C0POLYGON
  []
  [poly_right]
    type = ElementGenerator
    input = poly_left
    nodal_positions = '1 0 0   2 0 0   3 1 0   2 2 0   1 2 0   1 1 0'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = C0POLYGON
  []
  [repair]
    type = MeshRepairGenerator
    input = poly_right
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
