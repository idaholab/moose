[Mesh]
  [center]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  0 1 0  0 0 1e-07  1 0 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [face0]
    type = ElementGenerator
    input = center
    nodal_positions = '0 0 0  1 0 0  0 1 0  0 0 -1e-07  1 0 -1  0 1 -1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [face1]
    type = ElementGenerator
    input = face0
    nodal_positions = '0 0 0  1 0 0  0 -1 0  0 0 1e-07  1 0 1  0 -1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [face2]
    type = ElementGenerator
    input = face1
    nodal_positions = '1 1 0  1 0 0  0 1 0  1 1 1e-07  1 0 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [face3]
    type = ElementGenerator
    input = face2
    nodal_positions = '0 0 0  -1 0 0  0 1 0  0 0 1e-07  -1 0 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [face4]
    type = ElementGenerator
    input = face3
    nodal_positions = '-6.666666889e-08 -6.666666889e-08 6.666667556e-08  0.3333333111 -0.6666666889 0.6666667556  -0.6666666889 0.3333333111 0.6666667556  0 0 1e-07  1 -1.066522864e-16 1  -1.066522864e-16 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [node0]
    type = ElementGenerator
    input = face4
    nodal_positions = '0 0 0  -1 0 0  0 -1 0  0 0 -1e-07  -1 0 -1  0 -1 -1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [node1]
    type = ElementGenerator
    input = node0
    nodal_positions = '2 0 0  1 0 0  2 -1 0  2 0 -1e-07  1 0 -1  2 -1 -1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [node2]
    type = ElementGenerator
    input = node1
    nodal_positions = '0 2 0  -1 2 0  0 1 0  0 2 -1e-07  -1 2 -1  0 1 -1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [node3]
    type = ElementGenerator
    input = node2
    nodal_positions = '0 0 2e-07  -1 0 2e-07  0 -1 2e-07  0 0 1e-07  -1 0 -0.9999998  0 -1 -0.9999998'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [node4]
    type = ElementGenerator
    input = node3
    nodal_positions = '2 0 2  1 0 2  2 -1 2  2 0 1.9999999  1 0 1  2 -1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [node5]
    type = ElementGenerator
    input = node4
    nodal_positions = '0 2 2  -1 2 2  0 1 2  0 2 1.9999999  -1 2 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [repair]
    type = MeshRepairGenerator
    input = node5
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
