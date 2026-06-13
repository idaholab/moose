# A flat PYRAMID5 sliver over a thin (high-aspect, near-aligned) quad base shared with a thin
# HEX8. The pyramid is flagged as a sliver and absorbed into the (valid) thin hex.
[Mesh]
  [hex]
    type = ElementGenerator
    nodal_positions = '0  0.495  0  1  0.495  0  1  0.505  0  0  0.505  0  0  0.495  1  1  0.495  1  1  0.505  1  0  0.505  1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [pyr]
    type = ElementGenerator
    input = hex
    nodal_positions = '0  0.495  1  1  0.495  1  1  0.505  1  0  0.505  1  0.5  0.5  1.0002'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [repair]
    type = MeshRepairGenerator
    input = pyr
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
