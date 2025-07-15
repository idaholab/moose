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
  extra_tag_matrices = 'mass'
[]

[Functions]
  [b2]
    type = ParsedFunction
    expression = '-5*(x^2+y)*sin(t)-10*sin(t)'
  []
  [v_exact]
    type = ParsedFunction
    expression = '5*(x^2+y)*sin(t)'
  []
  [v_init]
    type = ParsedFunction
    expression = '-5*(x^2+y)*${dt_and_v0}'
  []
  [v_lr]
    type = ParsedFunction
    expression = '5*(1+y)*sin(t)'
  []
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [v_exact]
    [AuxKernel]
      type = FunctionAux
      function = v_exact
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
[]

[ICs]
  [v_old]
    type = FunctionIC
    variable = v
    state = OLD
    function = v_init
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
[]

[Postprocessors]
  [v_error]
    type = NodalL2Error
    variable = v
    function = v_exact
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
    use_constant_mass = true
    second_order_vars = 'v'
  []
[]

[Outputs]
  exodus = true
[]
