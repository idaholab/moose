mu = 1
rho = 1
k = 1e-3
cp = 1
alpha = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 50
    ny = 16
  []
[]

[Modules]
  [NavierStokesFV]
    # General parameters
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = true

    # Material properties
    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'

    # Initial conditions
    initial_velocity = '1 1 0'
    initial_pressure = 0.0
    initial_temperature = 0.0

    # Inlet boundary conditions
    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '1'

    # Wall boundary conditions
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    # Outlet boundary conditions
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    # Ambient convection volumetric heat source
    ambient_convection_alpha = 'alpha'
    ambient_temperature = '100'
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k rho mu alpha'
    prop_values = '${cp} ${k} ${rho} ${mu} ${alpha}'
  []
[]

[Postprocessors]
  [temp]
    type = ElementAverageValue
    variable = T_fluid
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
