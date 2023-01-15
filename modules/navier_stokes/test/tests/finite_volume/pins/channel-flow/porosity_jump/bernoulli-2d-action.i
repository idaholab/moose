rho=1.1
mu=0

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '0.5'
    ix = '3 3'
    iy = '2'
    subdomain_id = '1 2'
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true

    density = 'rho'
    dynamic_viscosity = 'mu'
    porosity = 'porosity'

    initial_velocity = '1 1e-6 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'slip slip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0.4'
    porosity_interface_pressure_treatment = 'bernoulli'
    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[AuxVariables]
  [porosity]
    type = PiecewiseConstantVariable
  []
[]

[ICs]
  [porosity_1]
    type = ConstantIC
    variable = porosity
    block = 1
    value = 1
  []
  [porosity_2]
    type = ConstantIC
    variable = porosity
    block = 2
    value = 0.5
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [inlet_p]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = superficial_vel_x
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
