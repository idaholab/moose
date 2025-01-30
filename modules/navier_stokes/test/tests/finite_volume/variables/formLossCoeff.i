[Mesh]
    [gen]
        type = CartesianMeshGenerator
        dim = 2
        # block_id = '1 2'
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

[Debug]
    show_material_props = true
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

    [pressVar]
      type = BernoulliPressureVariable
      pressure_drop_sideSets = 'area_change'
      pressure_drop_form_loss_coefficients = '0.40296'
      porosity = porosity
      u = u
      rho = 988.0
      # pressure = pressure
      initial_condition = 1.01e3
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
    [pressure_var_aux]
      type = FunctorMaterialRealAux
      functor = 'pressure'
      variable = 'pressVar'
      execute_on = 'initial timestep_end'
    []
  []
  
  [Materials]
    [all_constant_props]
      type = ADGenericConstantFunctorMaterial
      prop_names = 'u rho  mu   '
      prop_values = '1.2  988  1e-3 '
    []
    [porosityMat]
        type = ADPiecewiseByBlockFunctorMaterial
        prop_name = porosity
        subdomain_to_prop_value = '1 1.00
                                   2 0.25'
    []
    [DhMat]
      type = PiecewiseByBlockFunctorMaterial
      prop_name = characteristic_length
      subdomain_to_prop_value = '1  1.000
                                 2  0.500'
    []
    [churchill1]
      type = FunctorChurchillDragCoefficients
      # Dh_channel = ${fparse sqrt(4*1/pi)}
      # porosity = porosity_var
      block = 1
    []
    [churchill2]
        type = FunctorChurchillDragCoefficients
        # Dh_channel = ${fparse sqrt(4*0.25/pi)}
        # porosity = porosity_var
        block = 2
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
    momentum_inlet_functors = '1.2 0.0'
    momentum_outlet_types = fixed-pressure
    pressure_functors = '1.01e5'

    # Friction control parameters
    friction_types = 'darcy forchheimer'
    friction_coeffs = 'Darcy_coefficient Forchheimer_coefficient'
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
    [forchheimer]
      type = ElementAverageValue
      variable = forchheimer
    []
    [inlet_pressure]
        type = SideAverageValue
        variable = pressure
        boundary = left
        outputs = none
      []
      [outlet_pressure]
        type = SideAverageValue
        variable = pressure
        boundary = right
        outputs = none
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