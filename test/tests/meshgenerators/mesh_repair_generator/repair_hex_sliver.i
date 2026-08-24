# A flat (z-squashed) HEX8 sliver: its top face is 0.01 above the bottom, sandwiched between two
# HEX8s that share its top and bottom faces, with its four lateral faces on the boundary. The repair
# collapses the squashed pair of opposite faces so the two hexes meet, keeping a valid conformal
# mesh.
[Mesh]
  [flat_hex]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  1 1 0  0 1 0   0 0 0.01  1 0 0.01  1 1 0.01  0 1 0.01'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [add_bdies_on_flat_hex]
    type = SideSetsFromNormalsGenerator
    input = 'flat_hex'
    normals = '-1 0 0
               0 0 1
               0 0 -1'
    new_boundary = 'on_thin_side_should_be_lost on_quad_side_top on_quad_side_bot'
    output = true
  []
  [below]
    type = ElementGenerator
    nodal_positions = '0 0 -1  1 0 -1  1 1 -1  0 1 -1   0 0 0  1 0 0  1 1 0  0 1 0'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [add_bdies_on_kept_hexes_below]
    type = SideSetsFromNormalsGenerator
    input = 'below'
    normals = '0 0 1'
    new_boundary = 'on_quad_side_top_from_kept_hex'
  []
  [above]
    type = ElementGenerator
    nodal_positions = '0 0 0.01  1 0 0.01  1 1 0.01  0 1 0.01   0 0 1  1 0 1  1 1 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [add_bdies_on_kept_hexes_above]
    type = SideSetsFromNormalsGenerator
    input = 'above'
    normals = '0 0 -1'
    new_boundary = 'on_quad_side_bot_from_kept_hex'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'add_bdies_on_kept_hexes_below add_bdies_on_kept_hexes_above add_bdies_on_flat_hex'
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
