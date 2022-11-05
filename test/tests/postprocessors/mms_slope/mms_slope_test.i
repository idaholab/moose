[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]

#  do not use uniform refine, we are using adaptive refining
#  uniform_refine = 6
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'forcing_func u_func'

  [./forcing_func]
    type = ParsedFunction
    expression = alpha*alpha*pi*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '4'
  [../]

  [./u_func]
    type = ParsedGradFunction
    value = sin(alpha*pi*x)
    grad_x   = alpha*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '4'
  [../]
[]

[Kernels]
  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = '3'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = NEWTON

  nl_abs_tol = 1e-14

  [./Adaptivity]
    # if the refine fraction is 1 it will refine every element
    # remember < 1 means only refine that percentage of elements
    refine_fraction = 1

    steps = 6

    # do not coarsen at all
    coarsen_fraction = 0

    # maximum level of refinement steps, make sure this is > max_r_steps
    max_h_level = 10

    # leave this as is
    error_estimator = KellyErrorEstimator
  [../]
[]

# print l2 and h1 errors from the Postprocessors too so I can compare
[Postprocessors]
  active = 'l2_error h1_error dofs'
#  active = ' '

  [./l2_error]
    type = ElementL2Error
    variable = u
    function = u_func
  [../]

  [./h1_error]
    type = ElementH1Error
    variable = u
    function = u_func
  [../]

  [./dofs]
    type = NumDOFs
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  exodus = true
[]
