l = 5
inlet_area = 2
velocity_interp_method = 'rc'
advected_interp_method = 'average'

# Artificial fluid properties
# For a real case, use a GeneralFluidFunctorProperties and a viscosity rampdown
# or initialize very well!
k = 1
cp = 1000
mu = 1e2
rho = 1000

# Operating conditions
inlet_temp = 300
outlet_pressure = 1e5
inlet_velocity = 0.001

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${l} ${l}'
    dy = '${inlet_area}'
    ix = '5 5'
    iy = '2'
    subdomain_id = '1 2'
  []
  [side_set]
    type = SideSetsBetweenSubdomainsGenerator
    input = gen
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'mid-inlet'
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
    block = 2
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = ${inlet_velocity}
    block = 2
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
    block = 2
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${outlet_pressure}
    block = 2
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = ${inlet_temp}
    block = 2
  []
  [scalar]
    type = MooseVariableFVReal
    initial_condition = 0.1
    block = 2
  []
  [T_solid]
    type = MooseVariableFVReal
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
  # Mass equation
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  # X component momentum equation
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

  # Solid temperature
  [solid_temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T_solid
  []
[]

[FVBCs]
  # Inlet
  [inlet_mass]
    type = WCNSFVMassFluxBC
    variable = pressure
    boundary = 'mid-inlet'
    velocity_pp = 'inlet_velocity'
    area_pp = 'area_pp_left'
    rho = 'rho'
  []
  [inlet_u]
    type = WCNSFVMomentumFluxBC
    variable = vel_x
    boundary = 'mid-inlet'
    mdot_pp = 'inlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
    momentum_component = 'x'
  []
  [inlet_v]
    type = WCNSFVMomentumFluxBC
    variable = vel_y
    boundary = 'mid-inlet'
    mdot_pp = 0
    area_pp = 'area_pp_left'
    rho = 'rho'
    momentum_component = 'y'
  []
  [inlet_T]
    type = WCNSFVEnergyFluxBC
    variable = T_fluid
    boundary = 'mid-inlet'
    temperature_pp = 'inlet_T'
    velocity_pp = 'inlet_velocity'
    area_pp = 'area_pp_left'
    rho = 'rho'
    cp = 'cp'
  []
  [inlet_scalar]
    type = WCNSFVScalarFluxBC
    variable = scalar
    boundary = 'mid-inlet'
    scalar_value_pp = 'inlet_scalar_value'
    velocity_pp = 'inlet_velocity'
    area_pp = 'area_pp_left'
    rho = 'rho'
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
  [inlet_mdot]
    type = Receiver
    default = '${fparse 1980 * inlet_velocity * inlet_area}'
  []
  [inlet_velocity]
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

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k rho'
    prop_values = '${cp} ${k} ${rho}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = 'T_fluid'
    rho = ${rho}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'

  nl_abs_tol = 1e-9
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
[]
