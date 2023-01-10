l = 2
inlet_area = 2

# Artificial fluid properties
# For a real case, use a GeneralFluidFunctorProperties and a viscosity rampdown
# or initialize very well!
k = 1
cp = 1000
mu = 5e1
rho = 1000

# Operating conditions
inlet_temp = 300
outlet_pressure = 1e5
inlet_velocity = 0.2
inlet_scalar = 1.2

# The inlet angle, we will modify this and expect two things:
# 1. If we use a velocity postprocessor for the flux terms, we expect the mass flow
# to be proportional with "direction \cdot surface_normal".
# 2. If a mass flow is specified, it should not change, only the direction and magnitude of the
# inlet vleocity which is inferred based on the supplied massflow.
# direction = "0.86602540378 -0.5 0.0"
# direction = "1.0 0.0 0.0"
# cos_angle = 0.86602540378

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${inlet_area}
    nx = 10
    ny = 10
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'weakly-compressible'
    add_energy_equation = true
    add_scalar_equation = true
    passive_scalar_names = 'scalar'

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'
    passive_scalar_diffusivity = '10.0'
    passive_scalar_schmidt_number = '1.0'

    initial_velocity = '${inlet_velocity} 1e-15 0'
    initial_temperature = '${inlet_temp}'
    initial_pressure = '${outlet_pressure}'
    initial_scalar_variables = 1.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'flux-mass'
    flux_inlet_pps = 'inlet_mdot'
    energy_inlet_types = 'flux-mass'
    energy_inlet_function = 'inlet_T'
    passive_scalar_inlet_types = 'flux-mass'
    passive_scalar_inlet_function = 'inlet_scalar'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'slip slip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${outlet_pressure}'

    external_heat_source = 'power_density'
  []
[]

[Postprocessors]
  [inlet_mdot]
    type = Receiver
    default = '${fparse rho * inlet_velocity * inlet_area}'
  []
  [inlet_velocity]
    type = Receiver
    default = ${inlet_velocity}
  []
  [inlet_T]
    type = Receiver
    default = ${inlet_temp}
  []
  [inlet_scalar]
    type = Receiver
    default = ${inlet_scalar}
  []
  [outlet_mdot]
    type = VolumetricFlowRate
    advected_quantity = rho
    vel_x = vel_x
    vel_y = vel_y
    boundary = right
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
  [inlet_mdot_check]
    type = VolumetricFlowRate
    advected_quantity = rho
    vel_x = vel_x
    vel_y = vel_y
    boundary = left
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
  [inlet_vel_x_check]
    type = SideAverageValue
    variable = vel_x
    boundary = left
  []
  [inlet_vel_y_check]
    type = SideAverageValue
    variable = vel_y
    boundary = left
  []
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e4
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'rho cp k mu'
    prop_values = '${rho} ${cp} ${k} ${mu}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-9
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
[]

[Outputs]
  csv = true
[]
