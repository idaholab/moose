# A flat PYRAMID5 sliver (apex just above its quad base center) sitting on a HEX8, with four
# TET4 neighbors sharing its triangular cap faces. The repair dissolves the shared quad and
# absorbs the pyramid into the hex (-> a C0Polyhedron), leaving the four cap faces conformal.
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
  [tet0]
    type = ElementGenerator
    input = pyr
    nodal_positions = '0.5  0.5  1.01  0  0  1  1  0  1  0.5  0.5  2'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet1]
    type = ElementGenerator
    input = tet0
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
