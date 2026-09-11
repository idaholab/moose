# A flat-apex PYRAMID5 pancake on a HEX8, with one triangular cap face shared with a PRISM6 (the
# prism is extruded straight up from that cap face; the other three cap faces are external). The
# repair dissolves the pyramid's quad base and absorbs it into the hex (-> a C0Polyhedron); the
# shared cap face stays conformal against the prism.
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
  [repair]
    type = MeshRepairGenerator
    input = prism0
    fix_node_overlap = true
    fix_degenerate_elements = true
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
