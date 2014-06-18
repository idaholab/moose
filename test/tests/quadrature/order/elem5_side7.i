[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  nz = 0
  elem_type = QUAD4
[]

[Postprocessors]
  [./numsideqps]
    type = NumSideQPs
    boundary = 0
  [../]
  [./numelemqps]
    type = NumElemQPs
    block = 0
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
  [./Quadrature]
    order = third
    element_order = fifth
    side_order = seventh
  []
[]

[Outputs]
  output_initial = false
  exodus = false
  csv = true
  [./console]
    type = Console
    perf_log = false
  [../]
[]
