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
    elem_type = QUAD9
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = transient

    velocity_boundary = 'bottom right top             left'
    velocity_function = '0 0    0 0   lid_function 0  0 0'

    pressure_pinned_node = 0

    density_name = rho
    dynamic_viscosity_name = mu

    laplace = true
    family = LAGRANGE
    order = SECOND

    add_temperature_equation = true
    temperature_variable = T
    initial_temperature = 1
    thermal_conductivity_name = k
    specific_heat_name = cp
    natural_temperature_boundary = 'left right'
    fixed_temperature_boundary = 'top bottom'
    temperature_function = '0 1'
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
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
  type = Transient
  solve_type = 'NEWTON'
  # Run for 100+ timesteps to reach steady state.
  num_steps = 5
  dt = .5
  dtmin = .5
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'asm      2               ilu          4'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  file_base = lid_driven_out
  exodus = true
  perf_graph = true
[]
