[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = 2blocks3d.e
  patch_size = 5
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./penetration]
  [../]
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    value = t/10.0
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
  [./push_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 1
    function = horizontal_movement
  [../]
  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = '1 4'
    value = 0.0
  [../]
  [./fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = '1 4'
    value = 0.0
  [../]
[]

[Materials]
  [./left]
    type = Elastic
    block = 1
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    formulation = Nonlinear3D
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
  [./right]
    type = Elastic
    block = 2
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    formulation = Nonlinear3D
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Contact]
  [./leftright]
    secondary = 2
    primary = 3
    model = frictionless
    penalty = 1e+6
    normalize_penalty = true
    formulation = kinematic
    normal_smoothing_distance = 0.1
  [../]
[]

[Preconditioning]
  active = 'FSP'

  [./FSP]
    type = FSP
    # It is the starting point of splitting
    topsplit = 'contact_interior' # 'contact_interior' should match the following block name
    [./contact_interior]
      splitting          = 'contact interior'
      splitting_type     = multiplicative
    [../]
    [./interior]
      type = ContactSplit
      vars = 'disp_x disp_y disp_z'
      uncontact_primary   = '3'
      uncontact_secondary    = '2'
      uncontact_displaced = '1'
      blocks              = '1 2'
      include_all_contact_nodes = 1
      petsc_options_iname = '-ksp_type -ksp_max_it -ksp_rtol -ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter -pc_hypre_strong_threshold'
      petsc_options_value = ' preonly 10 1e-4 201                hypre    boomeramg      1                            0.25'
    [../]
    [./contact]
      type = ContactSplit
      vars = 'disp_x disp_y disp_z'
      contact_primary   = '3'
      contact_secondary    = '2'
      contact_displaced = '1'
      include_all_contact_nodes = 1
      petsc_options_iname = '-ksp_type -ksp_max_it -pc_type -pc_asm_overlap -sub_pc_type   -pc_factor_levels'
      petsc_options_value = '  preonly 10 asm  1 lu 0'
    [../]
  [../]
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  dt = 0.1
  dtmin = 0.1
  end_time = 0.1

  l_tol = 1e-4
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  nl_max_its = 100
[]

[Outputs]
  file_base = 2blocks3d_out
  [./exodus]
    type = Exodus
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]
