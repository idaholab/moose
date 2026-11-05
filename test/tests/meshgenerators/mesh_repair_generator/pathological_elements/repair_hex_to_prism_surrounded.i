[Mesh]
  [center]
    type = ElementGenerator
    nodal_positions = '0 0 0  1e-07 0 0  1 1 0  0 1 0  0 0 1  1e-07 0 1  1 1 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [face0]
    type = ElementGenerator
    input = center
    nodal_positions = '0 0 0  0 1 0  1 1 0  1e-07 0 0  0 0 -1  0 1 -1  1 1 -1  1e-07 0 -1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [face2]
    type = ElementGenerator
    input = face0
    nodal_positions = '1e-07 0 0  1 1 0  1 1 1  1e-07 0 1  1.0000001 0 0  2 1 0  2 1 1  1.0000001 0 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [face3]
    type = ElementGenerator
    input = face2
    nodal_positions = '1 1 0  0 1 0  0 1 1  1 1 1  1.44721356 1.894427209 0  0.4472135597 1.894427209 0  0.4472135597 1.894427209 1  1.44721356 1.894427209 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [face4]
    type = ElementGenerator
    input = face3
    nodal_positions = '0 1 0  0 0 0  0 0 1  0 1 1  -1 1 0  -1 0 0  -1 0 1  -1 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [face5]
    type = ElementGenerator
    input = face4
    nodal_positions = '0 0 1  1e-07 0 1  1 1 1  0 1 1  0 0 2  1e-07 0 2  1 1 2  0 1 2'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [node2]
    type = ElementGenerator
    input = face5
    nodal_positions = '1 1 0  1.5 1 0  1.5 1.5 0  1 1.5 0  1 1 -0.5  1.5 1 -0.5  1.5 1.5 -0.5  1 1.5 -0.5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [node3]
    type = ElementGenerator
    input = node2
    nodal_positions = '0 1 0  -0.5 1 0  -0.5 1.5 0  0 1.5 0  0 1 -0.5  -0.5 1 -0.5  -0.5 1.5 -0.5  0 1.5 -0.5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [node6]
    type = ElementGenerator
    input = node3
    nodal_positions = '1 1 1  1.5 1 1  1.5 1.5 1  1 1.5 1  1 1 1.5  1.5 1 1.5  1.5 1.5 1.5  1 1.5 1.5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [node7]
    type = ElementGenerator
    input = node6
    nodal_positions = '0 1 1  -0.5 1 1  -0.5 1.5 1  0 1.5 1  0 1 1.5  -0.5 1 1.5  -0.5 1.5 1.5  0 1.5 1.5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = node7
    fix_node_overlap = true
    fix_elements_orientation = true
    fix_pathological_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_element_volumes = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
