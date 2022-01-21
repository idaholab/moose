mu=1.1
rho=1.1

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
    simulation_type = 'steady-state'
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = false

    density_name = 'rho'
    dynamic_viscosity_name = 'mu'
    gravity = '0 0 0'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = 'velocity_x_inlet velocity_y_inlet'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Functions]
  [velocity_x_inlet]
    type = ParsedFunction
    value = '1'
  []
  [velocity_y_inlet]
    type = ParsedFunction
    value = '0'
  []
  [pressure_outlet]
    type = ParsedFunction
    value = '0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      200                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
