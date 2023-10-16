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

  [gen2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 3
    ymax = 4
    nx = 20
    ny = 10
  []
  [rename]
    type = RenameBlockGenerator
    input = gen2
    old_block = 0
    new_block = 1
  []

  [combine]
    type = CombinerGenerator
    inputs = 'gen rename'
  []
[]

[Physics]
  [flow]
    type = WCNSFVFlowPhysics
    compressibility = 'weakly-compressible'
    block = 0

    # velocity_names = 'vel_x vel_y'

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

  [flow_2]
    type = WCNSFVFlowPhysics
    compressibility = 'weakly-compressible'
    block = 1

    velocity_variable = 'u v'
    pressure_variable = 'p'

    density = 'rho_other'
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
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e4
  []
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = ${inlet_temp}
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
  [rho_other]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T_fluid
    pressure = p
    block = 1
    density_name = rho_other
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
