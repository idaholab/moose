rho = 'rho'
l = 10
velocity_interp_method = 'rc'
advected_interp_method = 'average'

# Artificial fluid properties
# For a real case, use a GeneralFluidFunctorProperties and a viscosity rampdown
k = 1
cp = 1000
mu = 1e2

# Operating conditions
inlet_temp = 100
outlet_pressure = 1e5
inlet_v = 1.0

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = 1
    nx = 10
    ny = 4
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
[]

[AuxVariables]
  [mixing_length]
    type = MooseVariableFVReal
  []
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e8
  []
[]

[FVKernels]
  inactive = 'u_turb v_turb temp_turb'
  [mass_time]
    type = WCNSFVMassTimeDerivative
    variable = pressure
    drho_dt = drho_dt
  []
  [mass]
    type = WCNSFVMassAdvection
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
  [u_turb]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_x
    rho = ${rho}
    mixing_length = 'mixing_length'
    momentum_component = 'x'
    u = vel_x
    v = vel_y
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
    momentum_component = 'y'
    mu = ${mu}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_turb]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_y
    rho = ${rho}
    mixing_length = 'mixing_length'
    momentum_component = 'y'
    u = vel_x
    v = vel_y
  []

  [temp_time]
    type = WCNSFVEnergyTimeDerivative
    variable = T_fluid
    rho = rho
    drho_dt = drho_dt
    h = h
    dh_dt = dh_dt
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
  [temp_turb]
    type = WCNSFVMixingLengthEnergyDiffusion
    variable = T_fluid
    rho = rho
    cp = cp
    mixing_length = 'mixing_length'
    schmidt_number = 1
    u = vel_x
    v = vel_y
  []
[]

[FVBCs]
  [slip_x]
    type = INSFVNaturalFreeSlipBC
    variable = vel_x
    boundary = 'top bottom'
    momentum_component = 'x'
  []
  [slip_y]
    type = INSFVNaturalFreeSlipBC
    variable = vel_y
    boundary = 'top bottom'
    momentum_component = 'y'
  []

  # Inlet
  [inlet_u]
    type = WCNSFVMomentumFluxDirichletBC
    variable = vel_x
    boundary = 'left'
    mdot_pp = 'inlet_mdot'
    area_pp = 'area_pp_left'
    rho = 'rho'
  []
  [inlet_v]
    type = WCNSFVMomentumFluxDirichletBC
    variable = vel_y
    boundary = 'left'
    mdot_pp = 0
    area_pp = 'area_pp_left'
    rho = 'rho'
  []
  [inlet_T]
    type = FVDirichletBC
    variable = T_fluid
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

[Functions]
  [k_func]
    type = ParsedFunction
    expression = '${k}'
  []
  [rho_func]
    type = ParsedFunction
    expression = '1.5e3 - x'
  []
  [mu_func]
    type = ParsedFunction
    expression = '${mu}'
  []
[]

[FluidProperties]
  # [fp]
  #   type = FlibeFluidProperties
  # []
  [fp]
    type = TemperaturePressureFunctionFluidProperties
    cv = ${cp}
    k = k_func
    rho = rho_func
    mu = mu_func
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
    temperature = T_fluid
    pressure = pressure
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T_fluid'
    rho = ${rho}
  []
[]

[AuxKernels]
  inactive = 'mixing_len'
  [mixing_len]
    type = WallDistanceMixingLengthAux
    walls = 'top'
    variable = mixing_length
    execute_on = 'initial'
    delta = 0.5
  []
[]

[Postprocessors]
  [inlet_mdot]
    type = Receiver
    default = 1.4e3
  []
  [velocity_pp]
    type = Receiver
    default = ${inlet_v}
  []
  [area_pp_left]
    type = AreaPostprocessor
    boundary = 'left'
    execute_on = 'INITIAL'
  []
  [inlet_mfr]
    type = VolumetricFlowRate
    boundary = left
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 'rho'
  []
  [outlet_mfr]
    type = VolumetricFlowRate
    boundary = right
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 'rho'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1.0
    optimal_iterations = 6
  []
  end_time = 1e2

  nl_abs_tol = 1e-9
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  compute_scaling_once = false
[]

[Outputs]
  csv = true
[]
