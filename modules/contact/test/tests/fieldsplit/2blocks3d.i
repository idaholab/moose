[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = 2blocks3d.e
  patch_size = 5
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
  [../]
[]

[AuxVariables]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    expression = t/10.0
  [../]
[]

[AuxKernels]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
    order = FIRST
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
  [./elasticity_tensor_left]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stress_left]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]

  [./elasticity_tensor_right]
    type = ComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stress_right]
    type = ComputeFiniteStrainElasticStress
    block = 2
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
      petsc_options_iname = '-ksp_type -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter -pc_hypre_boomeramg_strong_threshold'
      petsc_options_value = 'preonly   hypre    boomeramg      1                            0.25'
    [../]
    [./contact]
      type = ContactSplit
      vars = 'disp_x disp_y disp_z'
      contact_primary   = '3'
      contact_secondary    = '2'
      contact_displaced = '1'
      include_all_contact_nodes = 1
      petsc_options_iname = '-ksp_type -pc_type -pc_asm_overlap -sub_pc_type'
      petsc_options_value = 'preonly   asm      1               lu'
    [../]
  [../]
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
