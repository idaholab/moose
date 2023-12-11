n=64
mu=2e-3

[GlobalParams]
  gravity = '0 0 0'
  preset = true
  supg = false
[]

[Problem]
  extra_tag_matrices = 'mass'
  previous_nl_solution_required = true
  type = NavierStokesProblem
  mass_matrix = 'mass'
  schur_fs_index = '1'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = ${n}
    ny = ${n}
    elem_type = QUAD9
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]

  [./p]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]

  [x_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  []
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  [../]
  [x_mass]
    type = Mass
    variable = vel_x
    matrix_tags = 'mass'
  []

  [y_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  []
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  [../]
  [y_mass]
    type = Mass
    variable = vel_y
    matrix_tags = 'mass'
  []
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'top'
    function = 'lid_function'
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  # [./pressure_pin]
  #   type = DirichletBC
  #   variable = p
  #   boundary = 'pinned_node'
  #   value = 0
  # [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  ${mu}'
  [../]
[]

[Functions]
  [./lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    expression = '4*x*(1-x)'
  [../]
[]

[Preconditioning]
  active = 'FSP'
  [FSP]
    type = FSP
    topsplit = 'by_diri_others'
    [by_diri_others]
      splitting = 'diri others'
      splitting_type  = additive
      petsc_options_iname = '-ksp_type'
      petsc_options_value = 'preonly'
    []
      [diri]
        sides = 'left right top bottom'
        vars = 'vel_x vel_y'
        petsc_options_iname = '-pc_type'
        petsc_options_value = 'jacobi'
      []
      [others]
        splitting = 'u p'
        splitting_type  = schur
        petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_rtol -ksp_type -ksp_atol'
        petsc_options_value = 'full                            self                             300                1e-5      fgmres 1e-9'
        unside_by_var_boundary_name = 'left top right bottom left top right bottom'
        unside_by_var_var_name = 'vel_x vel_x vel_x vel_x vel_y vel_y vel_y vel_y'
      []
      [u]
          vars = 'vel_x vel_y'
          unside_by_var_boundary_name = 'left top right bottom left top right bottom'
          unside_by_var_var_name = 'vel_x vel_x vel_x vel_x vel_y vel_y vel_y vel_y'
          petsc_options = '-ksp_converged_reason'
          petsc_options_iname = '-pc_type -ksp_pc_side -ksp_type -ksp_rtol -pc_hypre_type -pc_hypre_boomeramg_grid_sweeps_up -pc_hypre_boomeramg_relax_type_all -pc_hypre_boomeramg_grid_sweeps_down -pc_hypre_boomeramg_relax_weight_all'
          petsc_options_value = 'hypre    right        gmres     1e-5      boomeramg      3                                  Jacobi                         2                                        0.5'
        []
        [p]
          vars = 'p'
          petsc_options = '-pc_lsc_scale_diag -ksp_converged_reason'
          petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side -pc_type  -lsc_pc_type -lsc_pc_hypre_type -lsc_ksp_type -lsc_ksp_rtol -lsc_ksp_pc_side -lsc_ksp_gmres_restart'
          petsc_options_value = 'fgmres     300                1e-5     lsc      right        lsc       hypre        boomeramg          gmres         1e-5          right            300'
        []
  []
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu       NONZERO'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  petsc_options_iname = '-snes_max_it'
  petsc_options_value = '100'
  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 6
    dt = 1e-2
  []
  steady_state_detection = true
[]

[Outputs]
  exodus = true
  checkpoint = true
[]
