rho = 'rho'
l = 10
inlet_area = 1
velocity_interp_method = 'rc'
advected_interp_method = 'average'

# Artificial fluid properties
# For a real case, use a GeneralFluidFunctorProperties and a viscosity rampdown
# or initialize very well!
k = 1
cp = 1000
mu = 1e2

# Operating conditions
inlet_temp = 300
outlet_pressure = 1e5
inlet_velocity = 0.001

end_time = 3.0
switch_time = 1.0

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = 1
    nx = 10
    ny = 5
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = ${inlet_velocity}
  []
  [v]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${outlet_pressure}
  []
  [T]
    type = INSFVEnergyVariable
    initial_condition = ${inlet_temp}
  []
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e4
  []
[]

[FVKernels]
  [mass_time]
    type = WCNSFVMassTimeDerivative
    variable = pressure
    drho_dt = drho_dt
  []
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_time]
    type = WCNSFVMomentumTimeDerivative
    variable = u
    drho_dt = drho_dt
    rho = rho
    momentum_component = 'x'
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [v_time]
    type = WCNSFVMomentumTimeDerivative
    variable = v
    drho_dt = drho_dt
    rho = rho
    momentum_component = 'y'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []

  [temp_time]
    type = WCNSFVEnergyTimeDerivative
    variable = T
    rho = rho
    drho_dt = drho_dt
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [heat_source]
    type = FVCoupledForce
    variable = T
    v = power_density
  []
[]

[FVBCs]
  # Inlet
  [inlet_u]
    type = WCNSFVSwitchableInletVelocityBC
    variable = u
    boundary = 'left'
    mdot_pp = 'inlet_mdot'
    area_pp = 'surface_inlet'
    rho = 'rho'
    switch_bc = true
    face_limiter = 1.0
  []
  [outlet_u]
    type = WCNSFVSwitchableInletVelocityBC
    variable = u
    boundary = 'right'
    mdot_pp = 'inlet_mdot'
    area_pp = 'surface_inlet'
    rho = 'rho'
    switch_bc = false
    scaling_factor = -1.0
    face_limiter = 1.0
  []

  [inlet_v]
    type = WCNSFVInletVelocityBC
    variable = v
    boundary = 'left'
    mdot_pp = 0
    area_pp = 'surface_inlet'
    rho = 'rho'
  []

  [inlet_T]
    type = WCNSFVInletTemperatureBC
    variable = T
    boundary = 'left'
    temperature_pp = 'inlet_T'
  []
  [outlet_T]
    type = NSFVOutflowTemperatureBC
    variable = T
    boundary = 'right'
    u = u
    v = v
    rho = 'rho'
    cp = 'cp'
    backflow_T = ${inlet_temp}
  []

  [outlet_p]
    type = INSFVSwitchableOutletPressureBC
    variable = pressure
    boundary = 'right'
    function = ${outlet_pressure}
    switch_bc = true
    face_limiter = 1.0
  []
  [inlet_p]
    type = INSFVSwitchableOutletPressureBC
    variable = pressure
    boundary = 'left'
    function = ${outlet_pressure}
    switch_bc = false
    face_limiter = 1.0
  []

  # Walls
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = u
    boundary = 'top bottom'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = v
    boundary = 'top bottom'
    function = 0
  []
[]

[Functions]
  [func_coef]
    type = ParsedFunction
    expression = 'if(t<${switch_time} | t>2.0*${switch_time}, 1, 0)'
  []
  [func_coef_comp]
    type = ParsedFunction
    expression = 'if(t<${switch_time} | t>2.0*${switch_time}, 0, 1)'
  []
  [mass_flux_and_pressure_test_scaling]
    type = ParsedFunction
    expression = 'if(t<${switch_time} | t>2.0*${switch_time}, 0.1, 0.2)'
  []
[]

[Controls]
  [func_control_u_inlet]
    type = BoolFunctionControl
    parameter = 'FVBCs/inlet_u/switch_bc'
    function = 'func_coef'
    execute_on = 'initial timestep_begin'
  []
  [func_control_u_outlet]
    type = BoolFunctionControl
    parameter = 'FVBCs/outlet_u/switch_bc'
    function = 'func_coef_comp'
    execute_on = 'initial timestep_begin'
  []
  [func_control_p_outlet]
    type = BoolFunctionControl
    parameter = 'FVBCs/outlet_p/switch_bc'
    function = 'func_coef'
    execute_on = 'initial timestep_begin'
  []
  [func_control_p_inlet]
    type = BoolFunctionControl
    parameter = 'FVBCs/inlet_p/switch_bc'
    function = 'func_coef_comp'
    execute_on = 'initial timestep_begin'
  []
  [func_control_limiter_u_inlet]
    type = RealFunctionControl
    parameter = 'FVBCs/inlet_u/face_limiter'
    function = 'mass_flux_and_pressure_test_scaling'
    execute_on = 'initial timestep_begin'
  []
  [func_control_limiter_u_outlet]
    type = RealFunctionControl
    parameter = 'FVBCs/outlet_u/face_limiter'
    function = 'mass_flux_and_pressure_test_scaling'
    execute_on = 'initial timestep_begin'
  []
  [func_control_limiter_p_outlet]
    type = RealFunctionControl
    parameter = 'FVBCs/outlet_p/face_limiter'
    function = 'mass_flux_and_pressure_test_scaling'
    execute_on = 'initial timestep_begin'
  []
  [func_control_limiter_p_inlet]
    type = RealFunctionControl
    parameter = 'FVBCs/inlet_p/face_limiter'
    function = 'mass_flux_and_pressure_test_scaling'
    execute_on = 'initial timestep_begin'
  []
[]

# used for the boundary conditions in this example
[Postprocessors]
  [inlet_mdot]
    type = Receiver
    default = '${fparse 1980 * inlet_velocity * inlet_area}'
  []
  [surface_inlet]
    type = AreaPostprocessor
    boundary = 'left'
    execute_on = 'INITIAL'
  []
  [inlet_T]
    type = Receiver
    default = ${inlet_temp}
  []
  [outlet_mfr]
    type = VolumetricFlowRate
    boundary = 'right'
    advected_quantity = 1.0
    vel_x = u
    vel_y = v
  []
[]

[FluidProperties]
  [fp]
    type = FlibeFluidProperties
  []
[]

[FunctorMaterials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k'
    prop_values = '${cp} ${k}'
  []
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T
    pressure = pressure
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T'
    rho = ${rho}
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  dt = 0.1
  end_time = ${end_time}

  nl_abs_tol = 1e-12
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
