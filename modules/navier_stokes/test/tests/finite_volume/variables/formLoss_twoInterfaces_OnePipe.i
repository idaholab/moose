# This deck does not converge when nodes 1 and 2 have a velocity of 10.0 and a porosity of 1.0 with node 3 have a porosity of 0.5. It runs but does not converge
# When the velocity is reduced to 1.0, the deck does run
# The deck also fails to converge when nodes 1 and 2 have a superficial velocity of 1.0 and a porosity of 0.5 while node 3 has a porosity of 1.0
[Mesh]
    [gen]
        type = CartesianMeshGenerator
        dim = 2
        # block_id = '1 2'
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

[Debug]
    show_material_props = true
[]
  [Variables]
    [pressVar]
      type = BernoulliPressureVariable
      pressure_drop_sidesets = 'area_change_1 area_change_2'
      pressure_drop_form_factors = '0.2973 0.25'
      porosity = porosity
      u = superficial_vel_x
      rho = 988.0
      # pressure = pressure
      initial_condition = 1.01e5
      block = '1 2 3 4 5'
  []
[]

#   [Problem]
#     solve = false
#   []

  [AuxVariables]
    [porosity_var]
      family = MONOMIAL
      order = CONSTANT
      initial_condition = 1
    []

    [forchheimer]
      family = MONOMIAL
      order = CONSTANT
    []
  []

  [AuxKernels]
    [porosity_var]
      type = FunctorAux
      functor = 'porosity'
      variable = 'porosity_var'
      execute_on = 'initial'
    []
    # [pressure_var_aux]
    #   type = FunctorMaterialRealAux
    #   functor = 'pressure'
    #   variable = 'pressVar'
    #   execute_on = 'initial timestep_end'
    # []
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
    # [DhMat]
    #   type = PiecewiseByBlockFunctorMaterial
    #   prop_name = characteristic_length
    #   subdomain_to_prop_value = '1  1.00
    #                              2  0.71
    #                              3  0.71
    #                              4  0.71
    #                              5  1.00'
    # []
    # [churchill1]
    #   type = FunctorChurchillDragCoefficients
    #   # Dh_channel = ${fparse sqrt(4*1/pi)}
    #   # porosity = porosity_var
    #   block = 1
    #   multipliers = '0.1 0.1 0.1'
    # []
    # [churchill2]
    #     type = FunctorChurchillDragCoefficients
    #     # Dh_channel = ${fparse sqrt(4*0.25/pi)}
    #     # porosity = porosity_var
    #     block = '2 3 4 5'
    #     multipliers = '0.1 0.1 0.1'
    # []
  []
[Modules]
[NavierStokesFV]
    block = '1 2 3 4 5'
    compressibility = 'weakly-compressible'
    porous_medium_treatment = true
    # add_energy_equation =
    # use_friction_correction = true
    # consistent_scaling = 1.0


     # Material property parameters
     density = rho
     dynamic_viscosity = mu
     pressure_variable = pressVar

     # Porous medium parameters
     porosity = porosity
     porosity_interface_pressure_treatment = 'bernoulli'

    #  #Boundary conditions
    #  inlet_boundaries = 'left'
    #  outlet_boundaries = 'right'

     # Boundary conditions
    inlet_boundaries = 'left'
    outlet_boundaries = 'right'
    momentum_inlet_types = fixed-velocity
    momentum_inlet_functors = '1.0 0.0'
    momentum_outlet_types = fixed-pressure
    pressure_functors = '1.01e5'

    # Friction control parameters
    # friction_types = 'darcy forchheimer'
    # friction_coeffs = 'Darcy_coefficient Forchheimer_coefficient'
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
    # Scaling.
    automatic_scaling = true
    off_diagonals_in_auto_scaling = true
    # compute_scaling_once = false
    line_search = none

  []

  [Postprocessors]
    [forchheimer]
      type = ElementAverageValue
      variable = forchheimer
    []
    [inlet_pressure]
      type = ElementAverageValue
      variable = pressVar
      block = 1
    []
    [middle_pressure]
      type = ElementAverageValue
      variable = pressVar
      block = 2
    []
    [outlet_pressure]
      type = ElementAverageValue
      variable = pressVar
      block = 5
    []
    [Dp1]
      type = ParsedPostprocessor
      pp_names = 'inlet_pressure middle_pressure'
      expression = 'inlet_pressure - middle_pressure'
    []
    [Dp2]
      type = ParsedPostprocessor
      pp_names = 'middle_pressure outlet_pressure'
      expression = 'middle_pressure - outlet_pressure'
    []
    [pressure_drop]
      type = ParsedPostprocessor
      pp_names = 'inlet_pressure outlet_pressure'
      expression = 'inlet_pressure - outlet_pressure'
    []
  []

  [Outputs]
    exodus = true
  []
