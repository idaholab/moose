# A flat PYRAMID5 sliver whose apex projects inside the base but offset toward a corner (a
# base-node-degenerate, asymmetric flat sliver). The apex must still project inside the quad
# base for the absorption to yield a valid cell.
[Mesh]
  [poly]
    type = ElementGenerator
    nodal_positions = '0  0  0  1  0  0  1  1  0  0  1  0  0  0  1  1  0  1  1  1  1  0  1  1'
    polygon_faces_connectivity = '0 3 2 1; 4 5 6 7; 0 1 5 4; 1 2 6 5; 2 3 7 6; 3 0 4 7'
    elem_type = C0POLYHEDRON
  []
  [pyr]
    type = ElementGenerator
    input = poly
    nodal_positions = '0  0  1  1  0  1  1  1  1  0  1  1  0.8  0.8  1.005'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [tet0]
    type = ElementGenerator
    input = pyr
    nodal_positions = '0.8  0.8  1.005  0  0  1  1  0  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet1]
    type = ElementGenerator
    input = tet0
    nodal_positions = '0.8  0.8  1.005  1  0  1  1  1  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet2]
    type = ElementGenerator
    input = tet1
    nodal_positions = '0.8  0.8  1.005  1  1  1  0  1  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet3]
    type = ElementGenerator
    input = tet2
    nodal_positions = '0.8  0.8  1.005  0  1  1  0  0  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = tet3
    fix_node_overlap = true
    fix_sliver_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
