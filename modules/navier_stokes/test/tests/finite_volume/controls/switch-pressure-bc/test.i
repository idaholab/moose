rho = 1
mu = 1
l = 1
velocity_interp_method = 'rc'
advected_interp_method = 'upwind'
outlet_pressure = 1e5
inlet_v = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = 1
    nx = 4
    ny = 2
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = ${inlet_v}
  []
  [vel_y]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${outlet_pressure}
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    momentum_component = 'y'
    mu = ${mu}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
[]

[FVBCs]
  [free_slip_x]
    type = INSFVNaturalFreeSlipBC
    variable = vel_x
    boundary = 'top bottom'
    momentum_component = 'x'
  []

  [free_slip_y]
    type = INSFVNaturalFreeSlipBC
    variable = vel_y
    boundary = 'top bottom'
    momentum_component = 'y'
  []

  # Inlet
  [inlet_u]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'left'
    function = ${inlet_v}
  []
  [inlet_u_later]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'right'
    function = ${fparse -1 * inlet_v}
    enable = false
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'left'
    function = 0
  []
  [inlet_v_later]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'right'
    function = 0
    enable = false
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'right'
    function = ${outlet_pressure}
  []
  [outlet_p_later]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'left'
    function = ${fparse 2 * outlet_pressure}
    enable = false
  []
[]

[Functions]
  [conditional_function]
    type = ParsedFunction
    expression = 't > 1.5'
  []
[]

[Controls]
  [p_threshold]
    type = ConditionalFunctionEnableControl
    conditional_function = conditional_function
    disable_objects = 'FVBCs::outlet_p FVBCs::inlet_u FVBCs::inlet_v'
    enable_objects = 'FVBCs::outlet_p_later FVBCs::inlet_u_later FVBCs::inlet_v_later'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Postprocessors]
  [pressure_right]
    type = SideAverageValue
    variable = pressure
    boundary = right
  []

  [pressure_left]
    type = SideAverageValue
    variable = pressure
    boundary = right
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  end_time = 3
  line_search = 'bt'
  nl_abs_tol = 1e-8
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
[]
