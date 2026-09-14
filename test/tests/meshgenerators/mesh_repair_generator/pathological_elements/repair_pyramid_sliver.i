# A PYRAMID5 sliver/needle: a spike with a tiny (1e-6) quad base and a full-height apex, so it is
# thin in two dimensions - negligible volume but not flat (its apex is far from the degenerate
# base). The repair removes it by collapsing its shortest edge. A separate healthy hexahedron is
# left untouched.
[Mesh]
  [needle]
    type = ElementGenerator
    nodal_positions = '0 0 0   1e-6 0 0   1e-6 1e-6 0   0 1e-6 0   5e-7 5e-7 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [healthy]
    type = ElementGenerator
    input = needle
    nodal_positions = '5 0 0   6 0 0   6 1 0   5 1 0   5 0 1   6 0 1   6 1 1   5 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = healthy
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
