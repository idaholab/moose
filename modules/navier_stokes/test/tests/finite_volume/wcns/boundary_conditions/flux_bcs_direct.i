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
    initial_condition = 1e-15
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${outlet_pressure}
  []
  [T]
    type = INSFVEnergyVariable
    initial_condition = ${inlet_temp}
  []
  [scalar]
    type = MooseVariableFVReal
    initial_condition = 0.2
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
    u = u
    v = v
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
    u = u
    v = v
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
    u = u
    v = v
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
    cp = cp
    rho = rho
    drho_dt = drho_dt
    dcp_dt = dcp_dt
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
    u = u
    v = v
    rho = ${rho}
  []
  [heat_source]
    type = FVCoupledForce
    variable = T
    v = power_density
  []

  # Scalar concentration equation
  [scalar_time]
    type = WCNSFVScalarTimeDerivative
    variable = scalar
    rho = rho
    drho_dt = drho_dt
  []
  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    u = u
    v = v
    rho = ${rho}
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
    area_pp = 'surface_inlet'
  []
  [inlet_u]
    type = WCNSFVMomentumFluxBC
    variable = u
    boundary = 'left'
    mdot_pp = 'inlet_mdot'
    area_pp = 'surface_inlet'
    rho = 'rho'
    momentum_component = 'x'
  []
  [inlet_v]
    type = WCNSFVMomentumFluxBC
    variable = v
    boundary = 'left'
    mdot_pp = 0
    area_pp = 'surface_inlet'
    rho = 'rho'
    momentum_component = 'y'
  []
  [inlet_T]
    type = WCNSFVEnergyFluxBC
    variable = T
    boundary = 'left'
    energy_pp = 'inlet_Edot'
    area_pp = 'surface_inlet'
  []
  [inlet_scalar]
    type = WCNSFVScalarFluxBC
    variable = scalar
    boundary = 'left'
    scalar_flux_pp = 'inlet_scalar_flux'
    area_pp = 'surface_inlet'
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

# used for the boundary conditions in this example
[Postprocessors]
  [inlet_mdot]
    type = Receiver
    default = ${fparse 1980 * inlet_velocity * inlet_area}
  []
  [surface_inlet]
    type = AreaPostprocessor
    boundary = 'left'
    execute_on = 'INITIAL'
  []
  [inlet_Edot]
    type = Receiver
    default = ${fparse 1980 * inlet_velocity * 2530 * inlet_temp * inlet_area}
  []
  [inlet_scalar_flux]
    type = Receiver
    default = ${fparse 1980 * inlet_velocity * 0.2 * inlet_area}
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = SimpleFluidProperties
      density0 = 1980
      cp = 2530
    []
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k dcp_dt'
    prop_values = '${cp} ${k} 0'
  []
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T
    pressure = pressure
  []
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    temperature = 'T'
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
