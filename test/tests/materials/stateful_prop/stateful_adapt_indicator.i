[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
[]

[Variables]
  [./u]
  [../]
[]

[Materials]
  [./stateful_prop]
    type = StatefulTest
    block = 0
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
    prop_name = thermal_conductivity
    prop_state = old # Use the "Old" value to compute conductivity
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
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]

[Adaptivity]

  marker = errorfrac
  max_h_level = 1

  [./Indicators]
    [./error]
      type = GradientJumpIndicator
      variable = 'u'
    [../]
  [../]

  [./Markers]
    [./errorfrac]
      type = ErrorFractionMarker
      refine = 0.6
      coarsen = 0.1
      indicator = error
    [../]xs
  [../]
[]
