[Mesh]
	file = sq-2blk.e
	uniform_refine = 3
[]

[Variables]
  # variable in the whole domain
  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]

  # subdomain restricted variable
  [./v]
    order = FIRST
    family = LAGRANGE
    block = '1'
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    # dudt = 3*t^2*(x^2 + y^2)
    value = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = t*t*t*((x*x)+(y*y))
  [../]

  [./exact_fn_v]
    type = ParsedFunction
    value = t+1
  [../]
[]

[Kernels]
  [./ie_u]
    type = TimeDerivative
    variable = u
  [../]

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./ffn_u]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]


  [./ie_v]
    type = TimeDerivative
    variable = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]

[]

[BCs]
  [./all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 2 3 4'
    function = exact_fn
  [../]

  [./bottom_v]
    type = DirichletBC
    variable = v
    boundary = 5
    value = 0
  [../]

  [./top_v]
    type = FunctionDirichletBC
    variable = v
    boundary = 6
    function = exact_fn_v
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'
  petsc_options = '-snes_mf_operator'

  start_time = 0.0
  num_steps = 5
  dt = 0.1
[]

[Output]
  file_base = out
  output_initial = false
  interval = 1
  exodus = true
  perf_log = true
[]

[Debug]
	show_var_residual = 'u v'
        show_var_residual_norms = true
[]
