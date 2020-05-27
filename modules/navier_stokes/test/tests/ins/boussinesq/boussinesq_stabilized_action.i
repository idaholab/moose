[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = .05
    ymax = .05
    nx = 20
    ny = 20
    elem_type = QUAD9
  []
[]


[Preconditioning]
  [Newton_SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'bjacobi  lu           NONZERO                   200'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  file_base = boussinesq_stabilized_out
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    gravity = '0 -9.81 0'

    velocity_boundary = 'bottom right top  left'
    velocity_function = '0 0    0 0   0 0  0 0'

    # Even though we are integrating by parts, because there are no integrated
    # boundary conditions on the velocity p doesn't appear in the system of
    # equations. Thus we must pin the pressure somewhere in order to ensure a
    # unique solution
    pressure_pinned_node = 0

    density_name = rho
    dynamic_viscosity_name = mu

    initial_velocity = '1e-15 1e-15 0'

    use_ad = true
    add_standard_velocity_variables_for_ad = false
    pspg = true
    supg = true

    family = LAGRANGE
    order = FIRST

    add_temperature_equation = true
    temperature_variable = temp
    temperature_scaling = 1e-4
    initial_temperature = 340
    thermal_conductivity_name = k
    specific_heat_name = cp
    natural_temperature_boundary = 'top bottom'
    fixed_temperature_boundary = 'left right'
    temperature_function = '300 400'

    boussinesq_approximation = true
    # material property for reference temperature does not need to be AD material property
    reference_temperature_name = temp_ref
    thermal_expansion_name = alpha
  []
[]

[Materials]
  [ad_const]
    type = ADGenericConstantMaterial
    # alpha = coefficient of thermal expansion where rho  = rho0 -alpha * rho0 * delta T
    prop_names =  'mu        rho   alpha   k        cp'
    prop_values = '30.74e-6  .5757 2.9e-3  46.38e-3 1054'
  []
  [const]
    type = GenericConstantMaterial
    prop_names =  'temp_ref'
    prop_values = '900'
  []
[]
