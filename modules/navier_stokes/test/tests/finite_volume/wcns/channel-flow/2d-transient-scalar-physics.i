l = 10

# Artificial fluid properties
# For a real case, use a GeneralFluidFunctorProperties and a viscosity rampdown
# or initialize very well!
k = 1
cp = 1000
mu = 1e2

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

[Physics]
  [flow]
    type = WCNSFVFlowPhysics
    compressibility = 'weakly-compressible'
    # porous_medium_treatment = true

    block = 0

    velocity_variable = 'vel_x vel_y'

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '${inlet_v} 1e-15 0'
    initial_pressure = '${outlet_pressure}'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${inlet_v} 0'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${outlet_pressure}'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
  [passive_scalar]
    type = WCNSFVScalarAdvectionPhysics
    compressibility = 'weakly-compressible'
    porous_medium_treatment = false

    block = 0

    coupled_flow_physics = flow

    velocity_variable = 'vel_x vel_y'
    passive_scalar_names = 'c1'

    density = 'rho'
    dynamic_viscosity = 'mu'
    passive_scalar_diffusivity = '3'
    passive_scalar_coupled_source = '4'
    passive_scalar_coupled_source_coeff = '1'

    initial_scalar_variables = '1'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${inlet_v} 0'
    passive_scalar_inlet_types = 'fixed-value'
    passive_scalar_inlet_function = '${inlet_temp}'

    passive_scalar_advection_interpolation = 'average'
  []
  [turbulence]
    type = WCNSFVTurbulencePhysics
    compressibility = 'weakly-compressible'
    porous_medium_treatment = true

    block = 0

    coupled_flow_physics = flow
    scalar_advection_physics = passive_scalar

    velocity_variable = 'vel_x vel_y'

    density = 'rho'
    dynamic_viscosity = 'mu'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${inlet_v} 0'
  []
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e4
  []
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 1
  []
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = 300
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
    prop_names = 'cp k mu'
    prop_values = '${cp} ${k} ${mu}'
  []
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T_fluid
    pressure = pressure
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

  nl_abs_tol = 1e-9
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  compute_scaling_once = false
[]

[Outputs]
  exodus = true
[]
