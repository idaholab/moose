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

dt_and_v0 = 0.00005

[Problem]
  extra_tag_matrices = 'mass'
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [dot_u]
  []
[]

[ICs]
  [u_old]
    type = ConstantIC
    variable = u
    state = OLD
    # set's v_0 to 1
    value = -${dt_and_v0}
  []
[]

[AuxKernels]
  [dot_u]
    type = TestNewmarkTI
    variable = dot_u
    displacement = u
    first = true
    execute_on = 'TIMESTEP_END'
  []
[]

[Functions]
  [exact_fn]
    type = ParsedFunction
    expression = 't + 0.5*t^2'
  []
  [exact_dot_fn]
    type = ParsedFunction
    expression = '1 + t'
  []
[]

[Kernels]
  [Mass_x]
    type = MassMatrix
    variable = u
    density = 1
    matrix_tags = 'mass'
  []
  [ffn]
    type = BodyForce
    variable = u
    function = 1
  []
[]

[Postprocessors]
  [l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  []
  [l2_dot_err]
    type = ElementL2Error
    variable = dot_u
    function = exact_dot_fn
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 20
  dt = ${dt_and_v0}
  l_tol = 1e-12
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'u'
  []
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    max_rows = 10
  []
[]
