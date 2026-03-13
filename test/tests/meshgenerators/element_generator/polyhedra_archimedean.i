dx = 3.5
phi = ${fparse (1 + sqrt(5)) / 2}
phi_inv = ${fparse 1./phi}
silver = ${fparse 1 + sqrt(2)}
one_ds = ${fparse 1 / silver}
a = 1.5
b = 2
c = 1.5

[Mesh]
  [truncated_tetrahedron]
    type = ElementGenerator
    nodal_positions = " ${fparse -sqrt(2) / 4} ${fparse -3*sqrt(2) / 4}  ${fparse -sqrt(2) / 4}
                        ${fparse 3*sqrt(2) / 4} ${fparse -sqrt(2) / 4}   ${fparse -sqrt(2) / 4}
                        ${fparse 3*sqrt(2) / 4} ${fparse sqrt(2) / 4}    ${fparse sqrt(2) / 4}
                        ${fparse sqrt(2) / 4}   ${fparse sqrt(2) / 4}    ${fparse 3*sqrt(2) / 4}
                        ${fparse -sqrt(2) / 4}  ${fparse -sqrt(2) / 4}   ${fparse 3*sqrt(2) / 4}
                        ${fparse sqrt(2) / 4}  ${fparse -3*sqrt(2) / 4}   ${fparse -sqrt(2) / 4}
                        ${fparse -3*sqrt(2) / 4} ${fparse -sqrt(2) / 4}  ${fparse -sqrt(2) / 4}
                        ${fparse -3*sqrt(2) / 4} ${fparse sqrt(2) / 4}   ${fparse sqrt(2) / 4}
                        ${fparse -sqrt(2) / 4} ${fparse 3*sqrt(2) / 4}    ${fparse -sqrt(2) / 4}
                        ${fparse sqrt(2) / 4}   ${fparse 3*sqrt(2) / 4}    ${fparse sqrt(2) / 4}
                        ${fparse -sqrt(2) / 4}  ${fparse -sqrt(2) / 4}   ${fparse -3*sqrt(2) / 4}
                        ${fparse sqrt(2) / 4}  ${fparse sqrt(2) / 4}   ${fparse -3*sqrt(2) / 4}"

    element_connectivity = '0 1 2 3 4 5;
                            0 5 6 7 11 10;
                            3 4 6 7 8 9;
                            10 11 8 9 2 1;
                            4 5 6;
                            7 8 11;
                            0 1 10;
                            7 8 11'
    elem_type = "C0POLYHEDRON"
    subdomain_name = 'truncated_tetrahedron'
  []

  [cuboctahedron]
    type = ElementGenerator
    input = 'truncated_tetrahedron'
    nodal_positions = '1 1 1
                       1 -1 1
                      -1 -1 1
                      -1  1 1
                      ${fparse sqrt(2)} 0 0
                      0 ${fparse sqrt(2)} 0
                      -${fparse sqrt(2)} 0 0
                      0 -${fparse sqrt(2)} 0
                       1  1 -1
                       1 -1 -1
                      -1 -1 -1
                      -1  1 -1'
    element_connectivity = '0 1 2 3;
                            0 1 4;
                            0 4 8 5;
                            0 5 3;
                            3 5 11 6;
                            3 6 2;
                            2 6 10 7;
                            2 7 1;
                            1 7 9 4;
                            8 9 4;
                            8 11 5;
                            10 11 6;
                            9 10 7;
                            8 9 10 11'
    elem_type = "C0POLYHEDRON"
    subdomain_name = 'cuboctahedron'
  []

  [truncated_cube]
    input = 'cuboctahedron'
    nodal_positions = '${one_ds} 1 1
                       1 ${one_ds} 1
                       1 -${one_ds} 1
                      ${one_ds} 1 1
                      ${one_ds} -1 1
                      -${one_ds} -1 1
                      -1 -${one_ds} 1
                      -1 ${one_ds} 1
                      -${one_ds} 1 1
                      -1 1 ${one_ds}
                      1  1 ${one_ds}
                      1 -1 ${one_ds}
                      -1 -1 ${one_ds}
                      -1 1 -${one_ds}
                      1 1 -${one_ds}
                      1 -1 -${one_ds}
                      -1 -1 -${one_ds}
                      ${one_ds} 1 -1
                      1 -${one_ds} -1
                       1 ${one_ds} -1
                      ${one_ds} 1 -1
                      ${one_ds} -1 -1
                      1 -${one_ds} -1
                      1 ${one_ds} -1
                      ${one_ds} 1 -1
                      -${one_ds} -1 -1
                      -1 -${one_ds} -1
                      -1 ${one_ds} -1
                      -${one_ds} 1 -1'
    element_connectivity = '0 1 2 3 4 5 6 7;
                            0 1 9;
                            6 7 8;
                            5 4 11;
                            3 2 10;
                            2 1 9 13 18 17 14 10;
                            4 3 10 14 16 20 15 11;
                            6 5 11 15 21 22 12 8;
                            0 7 8 12 23 19 13 9;
                            16 17 18 19 23 22 21 20'
    elem_type = "C0POLYHEDRON"
    subdomain_name = 'cuboctahedron'
  []

  [cubic_cubby]
    input = 'truncated_cube'
    nodal_positions = '-0.5 0.5 1
                       0.5 0.5 1
                       0.5 -0.5 1
                       -0.5 -0.5 1
                       0.5 -1 0.5
                       -0.5 -1 0.5
                       0.5 -1 -0.5
                       -0.5 -1 -0.5
                       0.5 -0.5 -1
                       -0.5 -0.5 -1
                       0.5 0.5 -1
                       -0.5 0.5 -1
                       0.5 1 -0.5
                       -0.5 1 -0.5
                       0.5 1 0.5
                       -0.5 1 0.5
                       -1 0.5 -0.5
                       -1 -0.5 -0.5
                       -1 0.5 0.5
                       -1 -0.5 0.5
                       1 -0.5 0.5
                       1 0.5 0.5
                       1 0.5 -0.5
                       1 -0.5 -0.5'
    element_connectivity = '0 1 2 3;
                            2 3 5 4;
                            4 5 7 6;
                            6 7 9 8;
                            8 9 11 10;
                            10 11 13 12;
                            12 13 15 14;
                            14 15 0 1;
                            1 2 20 21;
                            20 21 22 23;
                            22 23 8 10;
                            9 11 16 17;
                            16 17 19 18;
                            18 19 3 0;
                            5 7 17 19;
                            16 18 15 13;
                            12 14 21 22;
                            23 20 4 6;
                            18 15 0;
                            14 12 1;
                            2 4 20;
                            3 5 19;
                            9 7 17;
                            8 6 23;
                            10 22 12;
                            11 16 13'
    elem_type = "C0POLYHEDRON"
    subdomain_name = 'b2'
  []

  [convert]
    type = ElementsToTetrahedronsConverter
    input = 'cuboctahedron'
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
