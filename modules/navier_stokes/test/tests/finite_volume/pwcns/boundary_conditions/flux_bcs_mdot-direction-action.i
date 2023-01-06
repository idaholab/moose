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

# The inlet angle
direction = "0.86602540378 -0.5 0.0"
cos_angle = 0.86602540378

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

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'

    initial_velocity = '${inlet_velocity} 1e-15 0'
    initial_temperature = '${inlet_temp}'
    initial_pressure = '${outlet_pressure}'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'flux-velocity'
    flux_inlet_pps = 'inlet_velocity'
    flux_inlet_directions = '${direction}'
    energy_inlet_types = 'flux-velocity'
    energy_inlet_function = 'inlet_T'

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
    default = '${fparse rho * inlet_velocity * inlet_area * cos_angle}'
  []
  [inlet_velocity]
    type = Receiver
    default = ${inlet_velocity}
  []
  [inlet_T]
    type = Receiver
    default = ${inlet_temp}
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
    prop_names = 'rho cp k mu porosity'
    prop_values = '${rho} ${cp} ${k} ${mu} 0.5'
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

[Outputs]
  exodus = true
[]
