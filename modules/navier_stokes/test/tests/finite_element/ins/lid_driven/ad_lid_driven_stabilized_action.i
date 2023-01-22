[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 64
    ny = 64
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    velocity_boundary = 'bottom right top             left'
    velocity_function = '0 0    0 0   lid_function 0  0 0'

    pressure_pinned_node = 0

    density_name = rho
    dynamic_viscosity_name = mu

    initial_velocity = '1e-15 1e-15 0'

    use_ad = true
    pspg = true
    supg = true
    alpha = 0.1

    family = LAGRANGE
    order = FIRST
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1  1'
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  exodus = true
  file_base = lid_driven_stabilized_out
[]

[Postprocessors]
  [lin]
    type = NumLinearIterations
  []
  [nl]
    type = NumNonlinearIterations
  []
  [lin_tot]
    type = CumulativeValuePostprocessor
    postprocessor = 'lin'
  []
  [nl_tot]
    type = CumulativeValuePostprocessor
    postprocessor = 'nl'
  []
[]
