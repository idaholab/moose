# An interior (not boundary) sliver: a thin QUAD strip with a real neighbor on BOTH of its long
# edges. Absorbing the sliver into one neighbor must re-point its other (non-absorbed) long edge
# onto the promoted element so it stays shared with the opposite neighbor -- i.e. the surrounding
# elements remain conformal. Validated with MeshDiagnosticsGenerator (the result is a polygon, which
# Exodus cannot store).
[Mesh]
  [sliver]
    type = ElementGenerator
    nodal_positions = '0 0 0  2 0 0  2 -0.01 0  0 -0.01 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [top_neighbor]
    type = ElementGenerator
    input = sliver
    nodal_positions = '0 0 0  2 0 0  1 1 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
  [bottom_neighbor]
    type = ElementGenerator
    input = top_neighbor
    nodal_positions = '0 -0.01 0  2 -0.01 0  1 -1 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
  [repair]
    type = MeshRepairGenerator
    input = bottom_neighbor
    fix_node_overlap = true
    fix_sliver_triangles = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
