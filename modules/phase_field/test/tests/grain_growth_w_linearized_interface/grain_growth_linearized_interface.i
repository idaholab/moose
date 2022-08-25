[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 50
  ymax = 50
  nx = 10
  ny = 10
[]

[Variables]
  [phi0]
  []
  [phi1]
  []
[]

[AuxVariables]
  [gr0_aux]
  []
  [gr1_aux]
  []
  [bounds_dummy]
  []
[]

[AuxKernels]
  [gr0]
    type = LinearizedInterfaceAux
    variable = gr0_aux
    nonlinear_variable = phi0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [gr1]
    type = LinearizedInterfaceAux
    variable = gr1_aux
    nonlinear_variable = phi1
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ICs]
  [phi0_IC]
    type = SmoothCircleICLinearizedInterface
    variable = phi0
    invalue = 1.0
    outvalue = 0.0
    bound_value = 5.0
    radius = 30
    int_width = 10
    x1 = 0.0
    y1 = 0.0
    profile = TANH
  []
  [phi1_IC]
    type = SmoothCircleICLinearizedInterface
    variable = phi1
    invalue = 0.0
    outvalue = 1.0
    bound_value = 5.0
    radius = 30
    int_width = 10
    x1 = 0.0
    y1 = 0.0
    profile = TANH
  []
[]

[Kernels]
  #phi0 Kernels
  [phi0_dot]
    type = ChangedVariableTimeDerivative
    variable = phi0
    order_parameter = gr0
  []
  [phi0_ACInt]
    type = ACInterfaceChangedVariable
    variable = phi0
    kappa_name = kappa_op
    mob_name = L
    order_parameter = gr0
  []
  [gr0_AC]
    type = ACGrGrPolyLinearizedInterface
    variable = phi0
    mob_name = L
    this_op = gr0
    other_ops = gr1
    v = phi1
  []
  #phi1 Kernels
  [phi1_dot]
    type = ChangedVariableTimeDerivative
    variable = phi1
    order_parameter = gr1
  []
  [phi1_ACInt]
    type = ACInterfaceChangedVariable
    variable = phi1
    kappa_name = kappa_op
    mob_name = L
    order_parameter = gr1
  []
  [gr1_AC]
    type = ACGrGrPolyLinearizedInterface
    variable = phi1
    mob_name = L
    this_op = gr1
    other_ops = gr0
    v = phi0
  []
[]

[Materials]
  [gr0]
    type = LinearizedInterfaceFunction
    f_name = gr0
    phi = phi0
  []
  [gr1]
    type = LinearizedInterfaceFunction
    f_name = gr1
    phi = phi1
  []
  [GBEovlution]
    type = GBEvolution
    GBenergy = 0.97
    GBMobility = 0.6e-6
    T = 300
    wGB = 10
  []
[]

[Bounds]
  [phi0_upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = phi0
    bound_type = upper
    bound_value = 5.0
  []
  [phi0_lower_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = phi0
    bound_type = lower
    bound_value = -5.0
  []
  [phi1_upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = phi1
    bound_type = upper
    bound_value = 5.0
  []
  [phi1_lower_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = phi1
    bound_type = lower
    bound_value = -5.0
  []
[]

[Postprocessors]
  [grain_area_mat]
    type = ElementIntegralMaterialProperty
    mat_prop = gr0
    execute_on = 'initial TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -ksp_type -snes_type'
  petsc_options_value = 'bjacobi gmres vinewtonrsls'

  dt = 0.1
  end_time = 0.6
[]

[Outputs]
  exodus = true
[]
