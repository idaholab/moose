mu = 1.1
rho = 1.1

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2d-rc-action_out.e
    use_for_exodus_restart = true
  []
[]

[Debug]
  show_actions = true
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = false

    density = 'rho'
    dynamic_viscosity = 'mu'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'slip slip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    # initialize from mesh file
    initialize_variables_from_mesh_file = true
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
  type = Transient
  dt = 1
  num_steps = 2
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  csv = true
[]
