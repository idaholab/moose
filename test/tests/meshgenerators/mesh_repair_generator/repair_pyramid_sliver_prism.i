# Same flat-apex PYRAMID5 sliver on a HEX8, but one triangular cap face is shared with a
# PRISM6 (the other three with TET4s): the cap face stays conformal against the prism after
# the pyramid is absorbed into the hex.
[Mesh]
  [hex]
    type = ElementGenerator
    nodal_positions = '0  0  0  1  0  0  1  1  0  0  1  0  0  0  1  1  0  1  1  1  1  0  1  1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [pyr]
    type = ElementGenerator
    input = hex
    nodal_positions = '0  0  1  1  0  1  1  1  1  0  1  1  0.5  0.5  1.01'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [prism0]
    type = ElementGenerator
    input = pyr
    nodal_positions = '0.5  0.5  1.01  0  0  1  1  0  1  0.5  0.5  2  0  0  2  1  0  2'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [tet1]
    type = ElementGenerator
    input = prism0
    nodal_positions = '0.5  0.5  1.01  1  0  1  1  1  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet2]
    type = ElementGenerator
    input = tet1
    nodal_positions = '0.5  0.5  1.01  1  1  1  0  1  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet3]
    type = ElementGenerator
    input = tet2
    nodal_positions = '0.5  0.5  1.01  0  1  1  0  0  1  0.5  0.5  2'
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
