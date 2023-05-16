mu = 1
rho = 1

[Mesh]
  type = GeneratedMesh
  nx = 4
  ny = 4
  xmax = 3.9
  ymax = 4.1
  elem_type = TRI3
  dim = 2
[]

[Problem]
  coord_type = 'RZ'
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'

    density = 'rho'
    dynamic_viscosity = 'mu'
    porosity = 'porosity'

    initial_velocity = '1e-15 1e-15 0'
    initial_pressure = 0.0

    inlet_boundaries = 'bottom'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 1'
    wall_boundaries = 'left right'
    momentum_wall_types = 'symmetry noslip'
    outlet_boundaries = 'top'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    momentum_two_term_bc_expansion = true
    pressure_two_term_bc_expansion = true

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
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

[Postprocessors]
  [in]
    type = SideIntegralVariablePostprocessor
    variable = vel_y
    boundary = 'bottom'
  []
  [out]
    type = SideIntegralVariablePostprocessor
    variable = vel_y
    boundary = 'top'
  []
  [num_lin]
    type = NumLinearIterations
    outputs = 'console'
  []
  [num_nl]
    type = NumNonlinearIterations
    outputs = 'console'
  []
  [cum_lin]
    type = CumulativeValuePostprocessor
    outputs = 'console'
    postprocessor = 'num_lin'
  []
  [cum_nl]
    type = CumulativeValuePostprocessor
    outputs = 'console'
    postprocessor = 'num_nl'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
