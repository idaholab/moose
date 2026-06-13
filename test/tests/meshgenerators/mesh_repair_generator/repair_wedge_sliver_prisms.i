# A flat PRISM6 wedge sandwiched between two other PRISM6 elements that share its triangular end
# faces. The repair collapses the flat wedge so the two prisms meet, keeping the mesh conformal.
[Mesh]
  [wedge]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  0 1 0   0 0 0.01  1 0 0.01  0 1 0.01'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [below]
    type = ElementGenerator
    input = wedge
    nodal_positions = '0 0 -1  1 0 -1  0 1 -1   0 0 0  1 0 0  0 1 0'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [above]
    type = ElementGenerator
    input = below
    nodal_positions = '0 0 0.01  1 0 0.01  0 1 0.01   0 0 1  1 0 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [repair]
    type = MeshRepairGenerator
    input = above
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
