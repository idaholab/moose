[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  zmax = 0
  elem_type = QUAD9
[]

[Variables]
  [./diffused]
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[DiracKernels]
  [./foo]
    variable = diffused
    type = ConstantPointSource
    value = 1
    point = '0.3 0.3 0.0'
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom left right top'
    value = 0.0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Adaptivity]
  max_h_level = 2
  initial_steps = 2
  marker = marker
  steps = 2
  initial_marker = marker
  [./Indicators]
    [./indicator]
      type = GradientJumpIndicator
      variable = diffused
    [../]
  [../]
  [./Markers]
    [./marker]
      type = ErrorFractionMarker
      indicator = indicator
      refine = 0.5
    [../]
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  [./os2]
    type = Exodus
    refinements = 2
  [../]
  [./os4]
    type = Exodus
    refinements = 4
  [../]
[]
