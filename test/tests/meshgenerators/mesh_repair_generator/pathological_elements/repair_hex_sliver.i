# A HEX8 sliver: a thin column, thin in x and y (cross-section 1e-3) and full length in z. Two of
# its three opposite-face-pair separations are tiny, so it is thin in two dimensions. The repair
# collapses its two thin cross-sections onto its long axis, removing the column. A separate healthy
# hexahedron is left untouched.
[Mesh]
  [column]
    type = ElementGenerator
    nodal_positions = '0 0 0   1e-3 0 0   1e-3 1e-3 0   0 1e-3 0   0 0 1   1e-3 0 1   1e-3 1e-3 1   0 1e-3 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [healthy]
    type = ElementGenerator
    input = column
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
