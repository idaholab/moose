[Mesh]
  [dmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 40
    ny = 40
    xmax = 40
    ymax = 40
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./pid]
    order = CONSTANT
    family = monomial
  []
[]

[AuxKernels]
  [./pidaux]
    type = ProcessorIDAux
    variable = pid
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
  [../]

  [./dot]
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
  [./Periodic]
    [./x]
      variable = u
      primary = 'left'
      secondary = 'right'
      translation = '40 0 0'
    [../]

    [./y]
      variable = u
      primary = 'bottom'
      secondary = 'top'
      translation = '0 40 0'
    [../]
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
    boundary = right
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
  dt = 1
  num_steps = 5
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
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
