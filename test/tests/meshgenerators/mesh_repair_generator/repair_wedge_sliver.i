# A flat (axially squashed) PRISM6 wedge: its top triangle sits 0.01 above the bottom, sandwiched
# between two TET4s that share its triangular end faces, with its three quad sides on the boundary.
# The repair collapses the top triangle onto the bottom so the two tets meet, leaving a valid,
# conformal, all-tetrahedral mesh.
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
    nodal_positions = '0 0 0  0 1 0  1 0 0  0.25 0.25 -1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [above]
    type = ElementGenerator
    input = below
    nodal_positions = '0 0 0.01  1 0 0.01  0 1 0.01  0.25 0.25 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
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
