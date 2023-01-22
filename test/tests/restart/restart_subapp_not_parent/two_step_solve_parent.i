[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*t*(x*x+y*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = 2*t*(x*x+y*y)-4*t*t
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[ICs]
  active = ''
  [./u_var]
    type = FunctionIC
    variable = u
    function = exact_fn
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./average]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  start_time = 2.0
  end_time = 4.0
  dt = 1.0
[]

[MultiApps]
  [./full_solve]
    type = FullSolveMultiApp
    execute_on = initial
    positions = '0 0 0'
    # input file will come from cli-coupled_variables
  [../]
[]

[Transfers]
  [./transfer_u]
    type = MultiAppProjectionTransfer
    multi_app = full_solve
    direction = FROM_MULTIAPP
    variable = u
    source_variable = u
  [../]
[]

[Outputs]
  #file_base will come from cli-coupled_variables
  exodus = true
[]
