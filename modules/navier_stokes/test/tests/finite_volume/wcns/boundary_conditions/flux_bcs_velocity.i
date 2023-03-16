rho = 'rho'
l = 10
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
    velocity_pp = 'inlet_u'
    rho = 'rho'
  []
  [inlet_u]
    type = WCNSFVMomentumFluxBC
    variable = vel_x
    boundary = 'left'
    velocity_pp = 'inlet_u'
    rho = 'rho'
    momentum_component = 'x'
  []
  [inlet_v]
    type = WCNSFVMomentumFluxBC
    variable = vel_y
    boundary = 'left'
    velocity_pp = 0
    rho = 'rho'
    momentum_component = 'y'
  []
  [inlet_T]
    type = WCNSFVEnergyFluxBC
    variable = T_fluid
    boundary = 'left'
    velocity_pp = 'inlet_u'
    temperature_pp = 'inlet_T'
    rho = 'rho'
    cp = 'cp'
  []
  [inlet_scalar]
    type = WCNSFVScalarFluxBC
    variable = scalar
    boundary = 'left'
    scalar_value_pp = 'inlet_scalar_value'
    velocity_pp = 'inlet_u'
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'right'
    function = ${outlet_pressure}
  []

  # Walls
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top bottom'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'top bottom'
    function = 0
  []
[]

# used for the boundary conditions in this example
[Postprocessors]
  [inlet_u]
    type = Receiver
    default = ${inlet_velocity}
  []
  [area_pp_left]
    type = AreaPostprocessor
    boundary = 'left'
    execute_on = 'INITIAL'
  []
  [inlet_T]
    type = Receiver
    default = ${inlet_temp}
  []
  [inlet_scalar_value]
    type = Receiver
    default = 0.2
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
    prop_names = 'cp k'
    prop_values = '${cp} ${k}'
  []
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T_fluid
    pressure = pressure
  []
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
    dt = 1e-2
    optimal_iterations = 6
  []
  end_time = 1

  nl_abs_tol = 1e-9
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]
