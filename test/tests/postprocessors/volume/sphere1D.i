# The volume of each block should be 3

[Mesh]#Comment
  file = sphere1D.e
[] # Mesh

[Problem]
  coord_type = RSPHERICAL
[]

[Functions]
  [./fred]
    type = ParsedFunction
    value='200'
  [../]
[] # Functions

[AuxVariables]
  [./constantVar]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Variables]

  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  [../]

[] # Variables

[AuxKernels]
  [./fred]
    type = ConstantAux
    variable = constantVar
    block = 1
    value = 1
  [../]
[]

[Kernels]

  [./heat_r]
    type = Diffusion
    variable = u
  [../]

[] # Kernels

[BCs]

  [./temps]
    type = FunctionDirichletBC
    variable = u
    boundary = 1
    function = fred
  [../]

[] # BCs

[Materials]

[] # Materials

[Executioner]

  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  print_linear_residuals = true

  petsc_options_iname = '-pc_type -snes_ls -ksp_gmres_restart'
  petsc_options_value = 'lu       basic                 101'

  line_search = 'none'


  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10


  l_max_its = 20

[] # Executioner

[Postprocessors]
  [./should_be_one]
    type = ElementAverageValue
    block = 1
    variable = constantVar
  [../]
  [./volume1]
    type = VolumePostprocessor
    block = 1
    variable = constantVar
  [../]
  [./volume2]
    type = VolumePostprocessor
    block = 2
    variable = constantVar
  [../]
  [./volume3]
    type = VolumePostprocessor
    block = 3
    variable = constantVar
  [../]
[]

[Output]
  interval = 1
  output_initial = true
  exodus = true
  perf_log = true
[] # Output
