[Mesh]
  [gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '1.0 1.0 1.0 1.0 1.0 1.0'
      dy = '1.0'
      subdomain_id = '1 2 3 3 4 5'
  []
  [area_change_1]
    type = SideSetsAroundSubdomainGenerator
    input = gen
    block = 1
    normal = '1 0 0'
    new_boundary = 'area_change_1'
  []
  [area_change_2]
    type = SideSetsAroundSubdomainGenerator
    input = area_change_1
    block = 4
    normal = '1 0 0'
    new_boundary = 'area_change_2'
  []
[]

[Variables]
  [pressure]
    type = BernoulliPressureVariable
    pressure_drop_sidesets = 'area_change_1 area_change_2'
    pressure_drop_form_factors = '0.2973 0.25'
    porosity = porosity
    u = superficial_vel_x
    v = superficial_vel_y
    rho = 988.0
    initial_condition = 1.01e5
    block = '1 2 3 4 5'
  []
[]

[Materials]
  [all_constant_props]
    type = ADGenericConstantFunctorMaterial
    prop_names = 'u rho  mu   '
    prop_values = '0.50 988  1e-3 '
  []
  [porosityMat]
      type = ADPiecewiseByBlockFunctorMaterial
      prop_name = porosity
      subdomain_to_prop_value = '1 1.00
                                  2 0.50
                                  3 0.50
                                  4 0.50
                                  5 1.00'
  []
[]

[Modules]
  [NavierStokesFV]
    block = '1 2 3 4 5'
    compressibility = 'weakly-compressible'
    porous_medium_treatment = true

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
    momentum_inlet_functors = '1.0 0.0'
    momentum_outlet_types = fixed-pressure
    pressure_functors = '1.01e5'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-6
  l_max_its = 100
  nl_max_its = 100
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  line_search = none
[]

[Postprocessors]
  [inlet_pressure]
    type = ElementAverageValue
    variable = pressure
    block = 1
  []
  [middle_pressure]
    type = ElementAverageValue
    variable = pressure
    block = 2
  []
  [outlet_pressure]
    type = ElementAverageValue
    variable = pressure
    block = 5
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
