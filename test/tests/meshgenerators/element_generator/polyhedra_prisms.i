dx = 2

[Mesh]
  [triangular_prism]
    type = ElementGenerator
    nodal_positions = "0 0 0
                       1 0 0
                       0 1 0
                       0 0 1
                       1 0 1
                       0 1 1"

    element_connectivity = '0 2 1; 0 1 4 3; 1 2 5 4; 2 0 3 5; 3 4 5'
    elem_type = "C0POLYHEDRON"
    subdomain_name = 'triangular_prism'
  []
  # cube is covered as a platonic solid
  [pentagon_prism]
    type = ElementGenerator
    input = 'triangular_prism'
    nodal_positions = "${fparse dx + 0} -1 0
                       ${fparse dx + -1} 0 0
                       ${fparse dx + -1} 1 0
                       ${fparse dx + 1} 1 0
                       ${fparse dx + 1} 0 0
                       ${fparse dx + 0} -1 1
                       ${fparse dx + -1} 0 1
                       ${fparse dx + -1} 1 1
                       ${fparse dx + 1} 1 1
                       ${fparse dx + 1} 0 1"

    element_connectivity = '0 1 2 3 4; 0 1 6 5; 1 2 7 6; 2 3 8 7; 3 4 9 8; 4 0 5 9; 5 6 7 8 9'
    elem_type = "C0POLYHEDRON"
    subdomain_name = "pentagon_prism"
    subdomain_id = 1
  []
  [hexagon_prism]
    type = ElementGenerator
    input = 'pentagon_prism'
    nodal_positions = "${fparse 2*dx + 0} -2 0
                       ${fparse 2*dx + -1} -1 0
                       ${fparse 2*dx + -1} 1 0
                       ${fparse 2*dx + 0} 2 0
                       ${fparse 2*dx + 1} 1 0
                       ${fparse 2*dx + 1} -1 0
                       ${fparse 2*dx + 0} -2 1
                       ${fparse 2*dx + -1} -1 1
                       ${fparse 2*dx + -1} 1 1
                       ${fparse 2*dx + 0} 2 1
                       ${fparse 2*dx + 1} 1 1
                       ${fparse 2*dx + 1} -1 1"

    element_connectivity = '0 1 2 3 4 5; 0 1 7 6; 1 2 8 7; 2 3 9 8; 3 4 10 9; 4 5 11 10; 5 0 6 11; 6 7 8 9 10 11'
    elem_type = "C0POLYHEDRON"
    subdomain_name = "hexagon_prism"
    subdomain_id = 2
  []
  [heptagon_prism]
    type = ElementGenerator
    input = 'hexagon_prism'
    nodal_positions = "${fparse 3*dx + 0} -2 0
                       ${fparse 3*dx + -1} -1 0
                       ${fparse 3*dx + -1} 0 0
                       ${fparse 3*dx + -0.5} 0.5 0
                       ${fparse 3*dx + 0.5} 0.5 0
                       ${fparse 3*dx + 1} 0 0
                       ${fparse 3*dx + 1} -1 0
                       ${fparse 3*dx + 0} -2 1
                       ${fparse 3*dx + -1} -1 1
                       ${fparse 3*dx + -1} 0 1
                       ${fparse 3*dx + -0.5} 0.5 1
                       ${fparse 3*dx + 0.5} 0.5 1
                       ${fparse 3*dx + 1} 0 1
                       ${fparse 3*dx + 1} -1 1"

    element_connectivity = '0 1 2 3 4 5 6; 0 1 8 7; 1 2 9 8; 2 3 10 9; 3 4 11 10; 4 5 12 11; 5 6 13 12; 6 0 7 13; 7 8 9 10 11 12 13'
    elem_type = "C0POLYHEDRON"
    subdomain_name = "heptagon_prism"
    subdomain_id = 3
  []
  [octogon_prism]
    type = ElementGenerator
    input = 'heptagon_prism'
    nodal_positions = "${fparse 4*dx + 0.5} -1.5 0
                       ${fparse 4*dx + -0.5} -1.5 0
                       ${fparse 4*dx + -1} -1 0
                       ${fparse 4*dx + -1} 0 0
                       ${fparse 4*dx + -0.5} 0.5 0
                       ${fparse 4*dx + 0.5} 0.5 0
                       ${fparse 4*dx + 1} 0 0
                       ${fparse 4*dx + 1} -1 0
                       ${fparse 4*dx + 0.5} -1.5 1
                       ${fparse 4*dx + -0.5} -1.5 1
                       ${fparse 4*dx + -1} -1 1
                       ${fparse 4*dx + -1} 0 1
                       ${fparse 4*dx + -0.5} 0.5 1
                       ${fparse 4*dx + 0.5} 0.5 1
                       ${fparse 4*dx + 1} 0 1
                       ${fparse 4*dx + 1} -1 1"

    element_connectivity = '0 1 2 3 4 5 6 7; 0 1 9 8; 1 2 10 9; 2 3 11 10; 3 4 12 11; 4 5 13 12; 5 6 14 13; 6 7 15 14; 7 0 8 15; 8 9 10 11 12 13 14 15'
    elem_type = "C0POLYHEDRON"
    subdomain_name = "octogon_prism"
    subdomain_id = 4
  []

  [convert]
    type = ElementsToTetrahedronsConverter
    input = 'octogon_prism'
  []
  [check]
    type = MeshDiagnosticsGenerator
    input = 'convert'
    examine_element_overlap = WARNING
    examine_element_types = WARNING
    examine_element_volumes = WARNING
    examine_nonplanar_sides = INFO
    check_local_jacobian = WARNING
  []
[]
