
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
[]

[Variables]
  [./u]
  [../]

  [./disp_x]
  [../]

  [./disp_y]
  [../]
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
  [./diff_x]
    type = Diffusion
    variable = disp_x
  [../]

  [./diff_y]
    type = Diffusion
    variable = disp_y
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

  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = -0.01
  [../]

  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.01
  [../]

  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = -0.01
  [../]

  [./right_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0.01
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  initial_steps = 2
  steps = 1
  marker = marker
  initial_marker = marker
  max_h_level = 2
  [./Indicators]
    [./indicator]
      type = GradientJumpIndicator
      variable = u
    [../]
  [../]
  [./Markers]
    [./marker]
      type = ErrorFractionMarker
      indicator = indicator
      coarsen = 0.1
      refine = 0.7
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
