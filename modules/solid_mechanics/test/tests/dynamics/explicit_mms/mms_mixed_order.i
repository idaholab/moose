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

dt_and_v0 = 1e-5

[Problem]
  extra_tag_matrices = 'mass damping'
[]

[Functions]
  [b1]
    type = ParsedFunction
    expression = '5*(x+y^2)*(cos(t)-sin(t))'
  []
  [b2]
    type = ParsedFunction
    expression = '5*(x^2+y)*cos(t)-10*sin(t)'
  []
  [b3]
    type = ParsedFunction
    expression = '-15*(x^2+y^2)*sin(2*t)'
  []
  [u_exact]
    type = ParsedFunction
    expression = '5*(x+y^2)*sin(t)'
  []
  [v_exact]
    type = ParsedFunction
    expression = '5*(x^2+y)*sin(t)'
  []
  [w_exact]
    type = ParsedFunction
    expression = '5*(x^2+y^2)*sin(2*t)'
  []
  [udot_init]
    type = ParsedFunction
    expression = '-5*(x+y^2)*${dt_and_v0}'
  []
  [v_init]
    type = ParsedFunction
    expression = '-5*(x^2+y)*${dt_and_v0}'
  []
  [v_lr]
    type = ParsedFunction
    expression = '5*(1+y)*sin(t)'
  []
  [wdot_init]
    type = ParsedFunction
    expression = '-10*(x^2+y^2)*${dt_and_v0}'
  []
[]

[Variables]
  [u]
  []
  [v]
  []
  [w]
  []
[]

[AuxVariables]
  [u_exact]
    [AuxKernel]
      type = FunctionAux
      function = u_exact
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
  [v_exact]
    [AuxKernel]
      type = FunctionAux
      function = v_exact
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
  [w_exact]
    [AuxKernel]
      type = FunctionAux
      function = w_exact
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
[]

[ICs]
  [u_old]
    type = FunctionIC
    variable = u
    state = OLD
    function = udot_init
  []
  [v_old]
    type = FunctionIC
    variable = v
    state = OLD
    function = v_init
  []
  [w_old]
    type = FunctionIC
    variable = w
    state = OLD
    function = wdot_init
  []
[]

[BCs]
  [v_lr]
    type = ExplicitFunctionDirichletBC
    variable = v
    boundary = 'left right'
    function = v_lr
  []
[]

[Kernels]
  [u_mass]
    type = MassMatrix
    variable = u
    density = 1
    matrix_tags = 'mass'
  []
  [u_damping]
    type = MassMatrix
    variable = u
    density = 1
    matrix_tags = 'damping'
  []
  [u_ffn]
    type = BodyForce
    variable = u
    function = b1
  []
  [v_mass]
    type = MassMatrix
    variable = v
    density = 1
    matrix_tags = 'mass'
  []
  [v_diff]
    type = ADDiffusion
    variable = v
  []
  [v_ffn]
    type = BodyForce
    variable = v
    function = b2
  []
  [w_mass]
    type = MassMatrix
    variable = w
    density = 1
    matrix_tags = 'mass'
  []
  [w_reaction]
    type = ADReaction
    variable = w
    rate = 1
  []
  [w_ffn]
    type = BodyForce
    variable = w
    function = b3
  []
[]

[Postprocessors]
  [u_error]
    type = NodalL2Error
    variable = u
    function = u_exact
  []
  [v_error]
    type = NodalL2Error
    variable = v
    function = v_exact
  []
  [w_error]
    type = NodalL2Error
    variable = w
    function = w_exact
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 50
  dt = ${dt_and_v0}
  l_tol = 1e-12
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    damping_matrix_tag = 'damping'
    use_constant_mass = true
    use_constant_damping = true
    second_order_vars = 'u w'
    first_order_vars = 'u v'
  []
[]

[Outputs]
  exodus = true
[]
