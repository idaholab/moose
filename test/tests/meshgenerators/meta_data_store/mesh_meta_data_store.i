[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 12
    ny = 10
    xmin = 4
    xmax = 7
  []
[]

[Debug]
  show_mesh_meta_data = true
[]

[Variables]
  [./u]
  [../]
[]

[AutoLineSamplerTest]
  # Add a line sampler on the variable right at the nodes based on the GeneratedMeshGenerator
  variable = u
  mesh_generator = 'gmg'
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
