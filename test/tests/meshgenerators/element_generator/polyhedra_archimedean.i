dx = 3.5
phi = ${fparse (1 + sqrt(5)) / 2}
phi_inv = ${fparse 1./phi}
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

  [convert]
    type = ElementsToTetrahedronsConverter
    input = ''
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
