[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 16
    ny = 16
  []
[]

[AuxVariables]
  [u]
    initial_condition = 1
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    velocity_boundary = 'bottom right top             left'
    velocity_function = '0 0    0 0   lid_function 0  0 0'
    initial_velocity = '1e-15 1e-15 0'
    add_standard_velocity_variables_for_ad = false

    pressure_pinned_node = 0

    density_name = rho
    dynamic_viscosity_name = mu

    use_ad = true
    laplace = true
    family = LAGRANGE
    order = FIRST

    add_temperature_equation = true
    fixed_temperature_boundary = 'bottom top'
    temperature_function = '1 0'
    has_heat_source = true
    heat_source_var = u

    supg = true
    pspg = true
  []
[]


[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu cp k'
    prop_values = '1  1  1  .01'
  []
[]

[Functions]
  [lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    expression = '4*x*(1-x)'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_levels -ksp_gmres_restart'
  petsc_options_value = 'asm      6                     200'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_max_its = 6
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'u'
  []
[]
