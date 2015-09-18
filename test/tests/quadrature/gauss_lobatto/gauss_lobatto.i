[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  nz = 0
  elem_type = QUAD4
[]

[Postprocessors]
  [./num_elem_qps]
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

  # In 1D, 5th-order Gauss-Lobatto quadrature has 4 points, so in 2D
  # it should have 16.
  [./Quadrature]
    type = GAUSS_LOBATTO
    order = FIFTH
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = false
  csv = true
[]
