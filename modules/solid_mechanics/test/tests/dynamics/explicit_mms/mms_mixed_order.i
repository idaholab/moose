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

dt_and_v0 = 0.00001

[Problem]
  extra_tag_matrices = 'mass damping'
[]

[Functions]
  [b]
    type = ParsedFunction
    expression = '5*(x+y^2)*(cos(t)-sin(t))'
  []
  [u_exact]
    type = ParsedFunction
    expression = '5*(x+y^2)*sin(t)'
  []
  [v_init]
    type = ParsedFunction
    expression = '-5*(x+y^2)*${dt_and_v0}'
  []
[]

[Variables]
  [u]
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
[]

[ICs]
  [u_old]
    # type = ConstantIC
    type = FunctionIC
    variable = u
    state = OLD
    # set's v_0 to 1
    # value = -${dt_and_v0}
    function = v_init
  []
[]

[Kernels]
  [mass]
    type = MassMatrix
    variable = u
    density = 1
    matrix_tags = 'mass'
  []
  [damping]
    type = MassMatrix
    variable = u
    density = 1
    matrix_tags = 'damping'
  []
  [ffn]
    type = BodyForce
    variable = u
    function = b
  []
[]

[Postprocessors]
  [error]
    type = NodalL2Error
    variable = u
    function = u_exact
  []
[]

# [VectorPostprocessors]
#   [u]
#     type = NodalValueSampler
#     variable = u
#     sort_by = id
#     outputs = csv
#   []
#   [u_exact]
#     type = NodalValueSampler
#     variable = u_exact
#     sort_by = id
#     outputs = csv
#   []
# []

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
    second_order_vars = 'u'
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    file_base = './mms_mixed_order_output/'
    execute_vector_postprocessors_on = 'INITIAL TIMESTEP_END'
  []
[]
