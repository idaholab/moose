# The expected pressure increase from textbook correlations for an expansion is: -355.68 Pa
# The form loss coefficient is computed with the following expression:
#
# K = (1-\beta)^2 = 0.25,
#
# where $\beta$ is the area ratio (0.5 in our case). With this, the total pressure drop
# can be computed using:
#
# \Delta p = 0.5 * K * \rho * v_upstream^2 + 0.5 * \rho * (v_downstream^2 - v_upstream^2)
#
# The expected pressure drop from textbook correlations for a contraction is: 2980.03437 Pa
# The form loss coefficient is computed with the following expression:
#
# K = 0.5*(1-/beta)^0.75 = 0.29730,
#
# where $\beta$ is the area ratio (0.5 in our case). With this, the total pressure drop
# can be computed using:
#
# \Delta p = 0.5 * K * rho * v_downstream^2 + 0.5 * \rho * (v_downstream^2 - v_upstream^2)
#
# Every velocity in these expressions is interstitial normal velocity to the surface.

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

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        block = '1 2'
        compressibility = 'weakly-compressible'
        porous_medium_treatment = true

        # Material property parameters
        density = rho
        dynamic_viscosity = mu

        # Porous medium parameters
        porosity = porosity
        porosity_interface_pressure_treatment = 'bernoulli'
        pressure_drop_sidesets = 'area_change'
        pressure_drop_form_factors = '0.25'

        # Boundary conditions
        inlet_boundaries = 'left'
        outlet_boundaries = 'right'
        momentum_inlet_types = fixed-velocity
        momentum_inlet_functors = '0.6 0.0'
        momentum_outlet_types = fixed-pressure
        pressure_functors = '1.01e5'
      []
    []
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


