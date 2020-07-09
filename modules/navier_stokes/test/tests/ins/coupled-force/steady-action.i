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

[Variables]
  [u]
    family = LAGRANGE_VEC
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    velocity_boundary = 'bottom right top left'
    velocity_function = '0 0    0 0   0 0 0 0'
    add_standard_velocity_variables_for_ad = false

    pressure_pinned_node = 0

    density_name = rho
    dynamic_viscosity_name = mu

    use_ad = true
    laplace = true
    family = LAGRANGE
    order = FIRST

    supg = true
    pspg = true

    has_coupled_force = true
    coupled_force_var = u
  []
[]

[Kernels]
  [u_diff]
    type = VectorDiffusion
    variable = u
  []
[]

[BCs]
  [u_left]
    type = VectorFunctionDirichletBC
    variable = u
    boundary = 'left'
    function_x = 1
    function_y = 1
  []

  [u_right]
    type = VectorFunctionDirichletBC
    variable = u
    boundary = 'right'
    function_x = -1
    function_y = -1
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1  1'
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
  exodus = true
[]
