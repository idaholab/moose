[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[MeshModifiers]
  [./block]
    type = SubdomainBoundingBox
    bottom_left = '0.4 0.4 -1'
    top_right = '0.6 0.6 1'
    block_id = 1
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
    block = 1
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
