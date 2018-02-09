[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  ymax = 0.1
  xmax = 0.1
[]

[Variables]
  [./x]
  [../]
  [./y]
  [../]
[]

[Kernels]
  [./diff_y]
    type = Diffusion
    variable = y
  [../]
  [./diff_x]
    type = Diffusion
    variable = x
  [../]
[]

[BCs]
  [./right_x]
    type = PostprocessorDirichletBC
    variable = x
    boundary = 'right'
    postprocessor = incoming_x
  [../]
  [./left_y]
    type = DirichletBC
    variable = y
    boundary = 'left'
    value = 0
  [../]
  [./right_y]
    type = PostprocessorDirichletBC
    variable = y
    boundary = 'right'
    postprocessor = incoming_y
  [../]
  [./left_x]
    type = DirichletBC
    variable = x
    boundary = 'left'
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [./incoming_x]
    type = Receiver
    execute_on = 'TIMESTEP_BEGIN'
  [../]
  [./incoming_y]
    type = Receiver
    execute_on = 'TIMESTEP_BEGIN'
  [../]
[]

