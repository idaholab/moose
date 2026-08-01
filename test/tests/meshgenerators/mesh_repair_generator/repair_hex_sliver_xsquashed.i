# A HEX8 squashed in x (0.01 thick), between two HEX8s sharing its x-faces. The repair must identify
# the x-direction opposite-face pair (not z) as the squashed one and collapse it.
[Mesh]
  [flat]
    type = ElementGenerator
    nodal_positions = '0 0 0  0.01 0 0  0.01 1 0  0 1 0   0 0 1  0.01 0 1  0.01 1 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [left]
    type = ElementGenerator
    input = flat
    nodal_positions = '-1 0 0  0 0 0  0 1 0  -1 1 0   -1 0 1  0 0 1  0 1 1  -1 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [right]
    type = ElementGenerator
    input = left
    nodal_positions = '0.01 0 0  1 0 0  1 1 0  0.01 1 0   0.01 0 1  1 0 1  1 1 1  0.01 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = right
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
