[Mesh]
  [center]
    type = ElementGenerator
    nodal_positions = '0 0 0  1e-07 0 0  1 1 0  0 1 0  0.4 0.4 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [face0]
    type = ElementGenerator
    input = center
    nodal_positions = '0 0 0  1e-07 0 0  1 -0.724137931 0.6896551724  0 -0.724137931 0.6896551724  0.4 0.4 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [face1]
    type = ElementGenerator
    input = face0
    nodal_positions = '1.0000001e-07 -1e-07 6.000000602e-15  1e-07 0 0  1 1 0  1.0000001 6.883382753e-15 6.000000602e-08  0.4 0.4 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [face2]
    type = ElementGenerator
    input = face1
    nodal_positions = '0 1.470588235 0.8823529412  1e-07 1.470588235 0.8823529412  1 1 0  0 1 0  0.4 0.4 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [face3]
    type = ElementGenerator
    input = face2
    nodal_positions = '0 0 0  -7.24137931e-08 0 6.896551724e-08  -0.724137931 1 0.6896551724  0 1 0  0.4 0.4 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [face4]
    type = ElementGenerator
    input = face3
    nodal_positions = '0 0 0  1e-07 0 0  1 1 0  0 1 0  0.4 0.4 -1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [node0]
    type = ElementGenerator
    input = face4
    nodal_positions = '0 0 0  -1e-07 0 0  -1 -1 0  0 -1 0  -0.4 -0.4 -1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [node1]
    type = ElementGenerator
    input = node0
    nodal_positions = '2e-07 0 0  1e-07 0 0  -0.9999998 -1 0  2e-07 -1 0  -0.3999998 -0.4 -1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [node2]
    type = ElementGenerator
    input = node1
    nodal_positions = '2 2 0  1.9999999 2 0  1 1 0  2 1 0  1.6 1.6 -1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [node3]
    type = ElementGenerator
    input = node2
    nodal_positions = '0 2 0  -1e-07 2 0  -1 1 0  0 1 0  -0.4 1.6 -1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [node4]
    type = ElementGenerator
    input = node3
    nodal_positions = '0.8 0.8 2  0.7999999 0.8 2  -0.2 -0.2 2  0.8 -0.2 2  0.4 0.4 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [repair]
    type = MeshRepairGenerator
    input = node4
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
