dx = 3.5
phi = ${fparse (1 + sqrt(5)) / 2}
phi_inv = ${fparse 1./phi}
a = 1.5
b = 2
c = 1.5

# TODO: port the two new heuristics in t4 (dont check neighbors if low q, dont check at n_surrounding == 3
# TODO: test the 3 poly inputs

[Mesh]
  [tetrahedron]
    type = ElementGenerator
    nodal_positions = "${a} ${a} ${a}
                       ${a} -${a} -${a}
                       -${a} ${a} -${a}
                       -${a} -${a} ${a}"

    element_connectivity = '0 2 1; 0 1 3; 1 2 3; 2 0 3'
    elem_type = "C0POLYHEDRON"
  []

  [cube]
    type = ElementGenerator
    input = 'tetrahedron'
    nodal_positions = '${fparse dx + 0 -0.7 } ${fparse -b / 2} ${fparse -b / 2}
                       ${fparse dx + ${b} -0.7 } ${fparse -b / 2} ${fparse -b / 2}
                       ${fparse dx + ${b} -0.7 } ${fparse b / 2} ${fparse -b / 2}
                       ${fparse dx + 0 -0.7 } ${fparse b / 2} ${fparse -b / 2}
                       ${fparse dx + 0 -0.7 } ${fparse -b / 2} ${fparse b / 2}
                       ${fparse dx + ${b} -0.7 } ${fparse -b / 2} ${fparse b / 2}
                       ${fparse dx + ${b} -0.7 } ${fparse b / 2} ${fparse b / 2}
                       ${fparse dx + 0 -0.7 } ${fparse b / 2} ${fparse b / 2}'

    element_connectivity = '0 3 2 1;
                            0 1 5 4;
                            1 2 6 5;
                            2 3 7 6;
                            3 0 4 7;
                            4 5 6 7'
    elem_type = "C0POLYHEDRON"
    subdomain_id = 1
    subdomain_name = 'cube'
  []

  [octahedron]
    type = ElementGenerator
    input = 'cube'
    nodal_positions = '${fparse 2 * dx + c} 0 0
                       ${fparse 2 * dx + 0} ${c} 0
                       ${fparse 2 * dx + 0} ${fparse -c} 0
                       ${fparse 2 * dx -c} 0 0
                       ${fparse 2 * dx + 0} 0  ${c}
                       ${fparse 2 * dx + 0} 0 ${fparse -c}'

    element_connectivity = '4 0 1; 4 0 2; 4 1 3; 4 2 3;
                            5 0 1; 5 0 2; 5 1 3; 5 2 3'
    elem_type = "C0POLYHEDRON"
    subdomain_id = 2
    subdomain_name = 'octahedron'
  []

  [dodecahedron]
    type = ElementGenerator
    input = 'octahedron'
    nodal_positions = '${fparse 3 * dx + 1} 1 1
                       ${fparse 3 * dx + 1} 1 -1
                       ${fparse 3 * dx + 1} -1 1
                       ${fparse 3 * dx + 1} -1 -1
                       ${fparse 3 * dx - 1} 1 1
                       ${fparse 3 * dx - 1} 1 -1
                       ${fparse 3 * dx - 1} -1 1
                       ${fparse 3 * dx - 1} -1 -1
                       ${fparse 3 * dx + 0} ${phi} ${phi_inv}
                       ${fparse 3 * dx + 0} ${fparse -phi} ${phi_inv}
                       ${fparse 3 * dx + 0} ${phi} ${fparse -phi_inv}
                       ${fparse 3 * dx + 0} ${fparse -phi} ${fparse -phi_inv}
                       ${fparse 3 * dx + phi} ${phi_inv} 0
                       ${fparse 3 * dx - phi} ${phi_inv} 0
                       ${fparse 3 * dx + phi} ${fparse -phi_inv} 0
                       ${fparse 3 * dx - phi} ${fparse -phi_inv} 0
                       ${fparse 3 * dx + phi_inv} 0 ${phi}
                       ${fparse 3 * dx + phi_inv} 0 ${fparse -phi}
                       ${fparse 3 * dx - phi_inv} 0 ${fparse phi}
                       ${fparse 3 * dx - phi_inv} 0 ${fparse -phi}'

    element_connectivity = '2 9 6 18 16;
                            2 16 0 12 14;
                            14 12 1 17 3;
                            7 11 3 17 19;
                            7 11 9 6 15;
                            2 14 3 11 9;
                            15 13 4 18 6;
                            18 4 8 0 16;
                            0 8 10 1 12;
                            1 17 19 5 10;
                            19 5 13 15 7;
                            13 4 8 10 5'
    elem_type = "C0POLYHEDRON"
    subdomain_id = 3
    subdomain_name = 'dodecahedron'
  []

  [icosahedron]
     type = ElementGenerator
     input = 'dodecahedron'
     nodal_positions = '${fparse 4 * dx + 1} 0  ${phi}
                        ${fparse 4 * dx - 1} 0  ${phi}
                        ${fparse 4 * dx + 1} 0 -${phi}
                        ${fparse 4 * dx - 1} 0 -${phi}

                        ${fparse 4 * dx + phi}  -1 0
                        ${fparse 4 * dx + phi}  1 0
                        ${fparse 4 * dx + -phi} 1 0
                        ${fparse 4 * dx + -phi} -1 0

                        ${fparse 4 * dx + 0} -${phi} 1
                        ${fparse 4 * dx + 0}  ${phi} 1
                        ${fparse 4 * dx + 0} -${phi} -1
                        ${fparse 4 * dx + 0}  ${phi} -1
                        '

     element_connectivity = '0 8 4;
                             0 4 5;
                             0 5 9;
                             0 9 1;
                             0 1 8;

                             3 2 11;
                             3 11 6;
                             3 6 7;
                             3 7 10;
                             3 10 2;

                             5 11 9;
                             11 9 6;
                             6 9 1;
                             6 1 7;
                             7 1 8;
                             8 7 10;
                             8 10 4;
                             10 4 2;
                             4 2 5;
                             2 5 11'
     elem_type = "C0POLYHEDRON"
    subdomain_id = 4
    subdomain_name = 'icosahedron'
   []

  [convert]
    type = ElementsToTetrahedronsConverter
    input = 'icosahedron'
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

[Postprocessors]
  [vol]
    type = VolumePostprocessor
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  exodus = true
[]
