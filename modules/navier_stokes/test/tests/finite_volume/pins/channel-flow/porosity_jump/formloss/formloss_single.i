[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.0 1.0'
    dy = '1.0'
    subdomain_id = '1 2'
  []
  [area_change]
    type = SideSetsAroundSubdomainGenerator
    input = gen
    block = 1
    normal = '1 0 0'
    new_boundary = 'area_change'
  []
[]

[Variables]
  [pressure]
    type = BernoulliPressureVariable
    pressure_drop_sidesets = 'area_change'
    pressure_drop_form_factors = '0.25'
    porosity = porosity
    u = superficial_vel_x
    v = superficial_vel_y
    rho = 988.0
    initial_condition = 1.01e5
    block = '1 2'
  []
[]

[Materials]
  [all_constant_props]
    type = ADGenericConstantFunctorMaterial
    prop_names = 'rho  mu   '
    prop_values = '988  1e-3 '
  []
  [porosity]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.50
                               2 1.00'
  []
[]

[Modules]
  [NavierStokesFV]
    block = '1 2'
    compressibility = 'weakly-compressible'
    porous_medium_treatment = true
    add_energy_equation = false

    # Material property parameters
    density = rho
    dynamic_viscosity = mu
    pressure_variable = pressure

    # Porous medium parameters
    porosity = porosity
    porosity_interface_pressure_treatment = 'bernoulli'

    # Boundary conditions
    inlet_boundaries = 'left'
    outlet_boundaries = 'right'
    momentum_inlet_types = fixed-velocity
    momentum_inlet_functors = '0.6 0.0'
    momentum_outlet_types = fixed-pressure
    pressure_functors = '1.01e5'
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
  [inlet_pressure]
    type = ElementAverageValue
    variable = pressure
    block = 1
    outputs = none
  []
  [outlet_pressure]
    type = ElementAverageValue
    variable = pressure
    block = 2
    outputs = none
  []
  [pressure_drop]
    type = ParsedPostprocessor
    pp_names = 'inlet_pressure outlet_pressure'
    expression = 'inlet_pressure - outlet_pressure'
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]


