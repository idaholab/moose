mu = 1.1
rho = 1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = false

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '1 1 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'slip slip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
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

  [restart]
    type = Checkpoint
    additional_execute_on = 'FINAL'
  []
[]
