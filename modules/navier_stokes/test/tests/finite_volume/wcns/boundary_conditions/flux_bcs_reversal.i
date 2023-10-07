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
inlet_velocity = 0.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = 1
    nx = 6
    ny = 3
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
    initial_condition = ${inlet_velocity}
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${outlet_pressure}
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = ${inlet_temp}
  []
  [scalar]
    type = MooseVariableFVReal
    initial_condition = 0.1
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e6
  []
[]

[FVKernels]
  # Mass equation
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
    phi0 = 0.0
  []

  # X component momentum equation
  [u_time]
    type = WCNSFVMomentumTimeDerivative
    variable = vel_x
    drho_dt = drho_dt
    rho = rho
    momentum_component = 'x'
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

  # Y component momentum equation
  [v_time]
    type = WCNSFVMomentumTimeDerivative
    variable = vel_y
    drho_dt = drho_dt
    rho = rho
    momentum_component = 'y'
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
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  # Energy equation
  [temp_time]
    type = WCNSFVEnergyTimeDerivative
    variable = T_fluid
    cp = cp
    rho = rho
    drho_dt = drho_dt
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T_fluid
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [heat_source]
    type = FVCoupledForce
    variable = T_fluid
    v = power_density
  []

  # Scalar concentration equation
  [scalar_time]
    type = FVFunctorTimeKernel
    variable = scalar
  []
  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [scalar_diffusion]
    type = FVDiffusion
    variable = scalar
    coeff = 1.1
  []
  [scalar_source]
    type = FVBodyForce
    variable = scalar
    function = 2.1
  []
[]

[FVBCs]
  # Inlet
  [inlet_mass]
    type = WCNSFVMassFluxBC
    variable = pressure
    boundary = 'left'
    mdot_pp = 'inlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    vel_x = vel_x
    vel_y = vel_y
  []
  [inlet_u]
    type = WCNSFVMomentumFluxBC
    variable = vel_x
    boundary = 'left'
    mdot_pp = 'inlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    momentum_component = 'x'
    vel_x = vel_x
    vel_y = vel_y
  []
  [inlet_v]
    type = WCNSFVMomentumFluxBC
    variable = vel_y
    boundary = 'left'
    mdot_pp = 0
    area_pp = 'area_pp_left'
    rho = 'rho'
    momentum_component = 'y'
    vel_x = vel_x
    vel_y = vel_y
  []
  [inlet_T]
    type = WCNSFVEnergyFluxBC
    variable = T_fluid
    T_fluid = T_fluid
    boundary = 'left'
    temperature_pp = 'inlet_T'
    mdot_pp = 'inlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    cp = 'cp'
    vel_x = vel_x
    vel_y = vel_y
  []
  [inlet_scalar]
    type = WCNSFVScalarFluxBC
    variable = scalar
    boundary = 'left'
    scalar_value_pp = 'inlet_scalar_value'
    mdot_pp = 'inlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    vel_x = vel_x
    vel_y = vel_y
    passive_scalar = scalar
  []

  [outlet_mass]
    type = WCNSFVMassFluxBC
    variable = pressure
    boundary = 'right'
    mdot_pp = 'outlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    vel_x = vel_x
    vel_y = vel_y
  []

  [outlet_u]
    type = WCNSFVMomentumFluxBC
    variable = vel_x
    boundary = 'right'
    mdot_pp = 'outlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    momentum_component = 'x'
    vel_x = vel_x
    vel_y = vel_y
  []
  [outlet_v]
    type = WCNSFVMomentumFluxBC
    variable = vel_y
    boundary = 'right'
    mdot_pp = 0
    area_pp = 'area_pp_left'
    rho = 'rho'
    momentum_component = 'y'
    vel_x = vel_x
    vel_y = vel_y
  []

  [outlet_T]
    type = WCNSFVEnergyFluxBC
    variable = T_fluid
    T_fluid = T_fluid
    boundary = 'right'
    temperature_pp = 'inlet_T'
    mdot_pp = 'outlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    cp = 'cp'
    vel_x = vel_x
    vel_y = vel_y
  []

  [outlet_scalar]
    type = WCNSFVScalarFluxBC
    variable = scalar
    boundary = 'right'
    scalar_value_pp = 'inlet_scalar_value'
    mdot_pp = 'outlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    vel_x = vel_x
    vel_y = vel_y
    passive_scalar = scalar
  []

  # Walls
  [no_slip_x]
    type = INSFVNaturalFreeSlipBC
    variable = vel_x
    momentum_component = x
    boundary = 'top bottom'
  []
  [no_slip_y]
    type = INSFVNaturalFreeSlipBC
    variable = vel_y
    momentum_component = y
    boundary = 'top bottom'
  []
[]

# used for the boundary conditions in this example
[Postprocessors]
  [inlet_mdot]
    type = Receiver
    default = ${fparse 1980 * inlet_velocity * inlet_area}
    #outputs = none
  []
  [outlet_mdot]
    type = Receiver
    default = ${fparse -1980 * inlet_velocity * inlet_area}
    outputs = none
  []
  [area_pp_left]
    type = AreaPostprocessor
    boundary = 'left'
    execute_on = 'INITIAL'
    outputs = none
  []
  [inlet_T]
    type = Receiver
    default = ${inlet_temp}
    outputs = none
  []
  [inlet_scalar_value]
    type = Receiver
    default = 0.2
    outputs = none
  []

  [left_mdot]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = rho
    boundary = left
    #advected_interp_method = ${advected_interp_method}
  []

  [right_mdot]
    type = VolumetricFlowRate
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = rho
    boundary = right
    advected_interp_method = upwind #${advected_interp_method}
  []
[]

[FluidProperties]
  [fp]
    type = FlibeFluidProperties
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k drho_dt rho'
    prop_values = '${cp} ${k} 0 1980'
  []
  #[rho]
  #  type = RhoFromPTFunctorMaterial
  #  fp = fp
  #  temperature = T_fluid
  #  pressure = pressure
  #[]
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T_fluid'
    rho = ${rho}
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-1
    optimal_iterations = 6
    growth_factor = 4
  []
  end_time = 500000

  nl_abs_tol = 1e-7
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]
