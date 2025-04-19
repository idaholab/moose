[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Variables]
  [u_first]
  []
  [u_second]
  []
[]

[AuxVariables]
  [dot_u_second]

  []
[]

[AuxKernels]
  [dot_u_second]
    type = TestNewmarkTI
    variable = dot_u_second
    displacement = u_second
    first = true
    execute_on = 'TIMESTEP_END'
  []
[]

[Functions]
  [ic]
    type = ParsedFunction
    expression = 0
  []

  [forcing_fn_first]
    type = ParsedFunction
    expression = (x+y)
  []

  [exact_fn_first]
    type = ParsedFunction
    expression = t*(x+y)
  []

  [exact_fn_second]
    type = ParsedFunction
    expression = '0.5*t^2'
  []
  [exact_dot_fn_second]
    type = ParsedFunction
    expression = 't'
  []
[]

[Kernels]
  [Mass_x_second]
    type = MassMatrix
    variable = u_second
    density = 1
    matrix_tags = 'mass'
  []
  [ffn_second]
    type = BodyForce
    variable = u_second
    function = 1
  []
  [Mass_x_first]
    type = MassMatrix
    variable = u_first
    density = 1
    matrix_tags = 'mass'
  []
  [diff]
    type = Diffusion
    variable = u_first
  []
  [ffn]
    type = BodyForce
    variable = u_first
    function = forcing_fn_first
  []
[]

[BCs]
  [all]
    type = ExplicitFunctionDirichletBC
    variable = u_first
    boundary = '0 1 2 3'
    function = exact_fn_first
  []
[]

[Postprocessors]
  [l2_err_second]
    type = ElementL2Error
    variable = u_second
    function = exact_fn_second
  []
  [l2_dot_err_second]
    type = ElementL2Error
    variable = dot_u_second
    function = exact_dot_fn_second
  []
  [l2_err_first]
    type = ElementL2Error
    variable = u_first
    function = exact_fn_first
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 20
  dt = 0.00005
  l_tol = 1e-12
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'u_second'
    first_order_vars = 'u_first'
  []
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    max_rows = 10
  []
[]
