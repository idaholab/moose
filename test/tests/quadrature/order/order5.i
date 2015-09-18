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
    order = fifth
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = false
  csv = true
[]
