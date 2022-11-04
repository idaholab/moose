[Mesh]
  file = sq-2blk.e
  uniform_refine = 3
[]

[Variables]
  # variable in the whole domain
  [./u]
    order = CONSTANT
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]

  # subdomain restricted variable
  [./v]
    order = CONSTANT
    family = MONOMIAL
    block = '1'
  [../]
[]

[Functions]
  [./forcing_fn_u]
    type = ParsedFunction
    expression = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
  [../]

  [./forcing_fn_v]
    type = ParsedFunction
    expression = t
  [../]

  # [./exact_fn]
  #   type = ParsedFunction
  #   expression = t*t*t*((x*x)+(y*y))
  # [../]

  # [./exact_fn_v]
  #   type = ParsedFunction
  #   expression = t+1
  # [../]
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
    type = BodyForce
    variable = u
    function = forcing_fn_u
  [../]


  [./ie_v]
    type = TimeDerivative
    variable = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  [./ffn_v]
    type = BodyForce
    variable = v
    function = forcing_fn_v
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 5
  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_elem
  exodus = true
[]

[Debug]
  show_var_residual = 'u v'
  show_var_residual_norms = true
[]
