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
  [convert]
    type = ElementsToTetrahedronsConverter
    input = 'hexagon_prism'
  []
[]
