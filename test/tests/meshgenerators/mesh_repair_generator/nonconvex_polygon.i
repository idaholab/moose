dx = 2

[Mesh]
  [hex_one_nonconvex_dent]
    type = ElementGenerator
    nodal_positions = "0 -0.8 0
                       -1 -1 0
                       -1 1 0
                       0 2 0
                       1 1 0
                       1 -1 0"

    element_connectivity = '0 1 2 3 4 5'
    elem_type = "C0POLYGON"
  []
  [hex_two_nonconvex_dents]
    type = ElementGenerator
    input = hex_one_nonconvex_dent
    nodal_positions = "${fparse 2*dx + 0} -0.8 0
                       ${fparse 2*dx + -1} -1 0
                       ${fparse 2*dx + 0.01} 1 0
                       ${fparse 2*dx + 0} 2 0
                       ${fparse 2*dx + 1} 1 0
                       ${fparse 2*dx + 1} -1 0"

    element_connectivity = '0 1 2 3 4 5'
    elem_type = "C0POLYGON"
  []
  # looks like a 3-branch star
  [hex_three_nonconvex_dents]
    type = ElementGenerator
    input = hex_two_nonconvex_dents
    nodal_positions = "${fparse 2*dx + 0} -1 0
                       ${fparse 2*dx + -2} -1.5 0
                       ${fparse 2*dx - 0.01} 1 0
                       ${fparse 2*dx + 0} 2 0
                       ${fparse 2*dx + 0.01} 1 0
                       ${fparse 2*dx + 2} -1.5 0"

    element_connectivity = '0 1 2 3 4 5'
    elem_type = "C0POLYGON"
  []
  [hex_two_nodes_in_a_single_nonconvex_dent]
    type = ElementGenerator
    input = hex_three_nonconvex_dents
    nodal_positions = "${fparse 2*dx + 0} -2 0
                       ${fparse 2*dx + 0.01} -1 0
                       ${fparse 2*dx + 0.01} 1 0
                       ${fparse 2*dx + 0} 2 0
                       ${fparse 2*dx + 1} 1 0
                       ${fparse 2*dx + 1} -1 0"

    element_connectivity = '0 1 2 3 4 5'
    elem_type = "C0POLYGON"
  []

  [check]
    type = MeshDiagnosticsGenerator
    input = hex_two_nodes_in_a_single_nonconvex_dent
    check_polygons = INFO
  []
  [repair]
    type = MeshRepairGenerator
    input = 'check'
    split_nonconvex_polygons = true
  []
  [check_after_repair]
    type = MeshDiagnosticsGenerator
    input = repair
    check_polygons = ERROR
  []
  # TODO: convert to triangles for exodus output
[]