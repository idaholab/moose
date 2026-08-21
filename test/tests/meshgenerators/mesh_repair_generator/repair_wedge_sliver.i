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
  [add_bdies_on_wedge]
    type = SideSetsFromNormalsGenerator
    input = 'wedge'
    normals = '-1 0 0
               0 0 1
               0 0 -1'
    new_boundary = 'on_quad_side_should_be_lost on_tri_side_top on_tri_side_bot'
    output = true
  []
  [below]
    type = ElementGenerator
    nodal_positions = '0 0 0  0 1 0  1 0 0  0.25 0.25 -1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [add_bdies_on_tet_bot]
    type = SideSetsFromNormalsGenerator
    input = 'below'
    normals = '0 0 1'
    new_boundary = 'on_tri_side_bot_from_tri'
  []
  [above]
    type = ElementGenerator
    nodal_positions = '0 0 0.01  1 0 0.01  0 1 0.01  0.25 0.25 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [add_bdies_on_tet_top]
    type = SideSetsFromNormalsGenerator
    input = 'above'
    normals = '0 0 -1'
    new_boundary = 'on_tri_side_top_from_tri'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'add_bdies_on_tet_bot add_bdies_on_tet_top add_bdies_on_wedge'
    avoid_merging_boundaries = true
  []
  [repair]
    type = MeshRepairGenerator
    input = combined
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
