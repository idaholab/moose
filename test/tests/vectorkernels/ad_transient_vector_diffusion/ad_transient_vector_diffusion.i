[Mesh]
  [./generator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  [../]
  [./block1]
    type = SubdomainBoundingBoxGenerator
    input = generator
    bottom_left = '0 0 -1'
    top_right = '1 1 1'
    block_id = 1
  [../]
  [./block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    bottom_left = '0.33 0.33 -1'
    top_right = '0.67 0.67 1'
    block_id = 2
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
  [../]
[]

[ICs]
  [./u]
    type = VectorConstantIC
    variable = u
    x_value = 1
    y_value = 2
    z_value = 3
    block = 2
  [../]
[]

[Kernels]
  [./diff]
    type = ADVectorDiffusion
    variable = u
  [../]
  [./time]
    type = ADVectorTimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.01
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
