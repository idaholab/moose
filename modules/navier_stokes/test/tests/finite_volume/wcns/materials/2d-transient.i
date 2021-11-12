l = 10
vel = 'velocity'
velocity_interp_method = 'rc'
advected_interp_method = 'average'

# Operating conditions
inlet_temp = 300
outlet_pressure = 1e5
inlet_v = 0.001

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = 1
    nx = 20
    ny = 10
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = ${inlet_v}
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
[]

[AuxVariables]
  [velocity_norm]
    type = MooseVariableFVReal
  []
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
    vel = ${vel}
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    u = u
    v = v
    pressure = pressure
    mu = 'mu'
    rho = 'rho'
  []

  [u_time]
    type = WCNSFVMomentumTimeDerivative
    variable = u
    drho_dt = drho_dt
    rho = rho
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = ${vel}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = 'mu'
    rho = 'rho'
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = 'mu'
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
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = ${vel}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = 'mu'
    rho = 'rho'
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = 'mu'
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
    vel = ${vel}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = 'mu'
    rho = 'rho'
  []
  [heat_source]
    type = FVCoupledForce
    variable = T
    v = power_density
  []
[]

[FVBCs]
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

  # Inlet
  [inlet_u]
    type = INSFVInletVelocityBC
    variable = u
    boundary = 'left'
    function = ${inlet_v}
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    variable = v
    boundary = 'left'
    function = 0
  []
  [inlet_T]
    type = FVDirichletBC
    variable = T
    boundary = 'left'
    value = ${inlet_temp}
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'right'
    function = ${outlet_pressure}
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = FlibeFluidProperties
    []
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    temperature = 'T'
    rho = 'rho'
  []
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = 'pressure'
    T_fluid = 'T'
    speed = 'velocity_norm'

    # To initialize with a high viscosity
    mu_rampdown = 'mu_rampdown'

    # For porous flow
    characteristic_length = 1
    porosity = 1
  []
[]

[AuxKernels]
  [speed]
    type = VectorMagnitudeAux
    variable = 'velocity_norm'
    x = u
    y = v
  []
[]

[Functions]
  [mu_rampdown]
    type = PiecewiseLinear
    x = '1 2 3 4'
    y = '1e3 1e2 1e1 1'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-3
    optimal_iterations = 6
  []
  end_time = 15

  nl_abs_tol = 1e-12
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
[]

[Outputs]
  exodus = true
[]
