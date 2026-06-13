# A blade PRISM6 wedge whose longest quad side is shared with another PRISM6 (rather than a hex).
# The repair absorbs the blade into that prism neighbor -> a C0Polyhedron, keeping the mesh
# conformal.
[Mesh]
  [wedge]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  0.5 0.01 0   0 0 1  1 0 1  0.5 0.01 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [prism]
    type = ElementGenerator
    input = wedge
    nodal_positions = '0 0 0  0.5 -1 0  1 0 0   0 0 1  0.5 -1 1  1 0 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [repair]
    type = MeshRepairGenerator
    input = prism
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
