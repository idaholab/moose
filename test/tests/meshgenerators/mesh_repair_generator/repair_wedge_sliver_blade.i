# A thin-cross-section PRISM6 wedge (a "blade"): its triangular cross-section is a sliver (the third
# vertex lies within 0.01 of the opposite edge, of length 1) but it has full height. Its longest
# quad side is shared with a HEX8. The repair absorbs the blade into the hex -> a C0Polyhedron,
# keeping the mesh conformal.
[Mesh]
  [wedge]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  0.5 0.01 0   0 0 1  1 0 1  0.5 0.01 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [hex]
    type = ElementGenerator
    input = wedge
    nodal_positions = '0 -1 0  1 -1 0  1 0 0  0 0 0   0 -1 1  1 -1 1  1 0 1  0 0 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = hex
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
