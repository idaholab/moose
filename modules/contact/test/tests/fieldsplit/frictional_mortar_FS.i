offset = 0.021

vy = 0.15
vx = 0.04

refine = 1

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [./original_file_mesh]
    type = FileMeshGenerator
    file = long_short_blocks.e
  [../]
  uniform_refine =  ${refine}
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    block = '1 2'
  [../]
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    value = 'if(t<0.5,${vx}*t-${offset},${vx}-${offset})'
  [../]
  [./vertical_movement]
    type = ParsedFunction
    value = 'if(t<0.5,${offset},${vy}*(t-0.5)+${offset})'
  [../]
[]

[BCs]
  [./push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30
    function = horizontal_movement
  [../]
  [./fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  [../]
  [./fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = '40'
    value = 0.0
  [../]
  [./push_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '30'
    function = vertical_movement
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
  [leftright]
    mesh = original_file_mesh
    secondary = 10
    master = 20

    model = coulomb
    formulation = mortar

    friction_coefficient = 0.2
  [../]
[]

[ICs]
  [./disp_y]
    block = 1
    variable = disp_y
    value = ${offset}
    type = ConstantIC
  [../]
  [./disp_x]
    block = 1
    variable = disp_x
    value = -${offset}
    type = ConstantIC
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
      vars = 'disp_x disp_y'
      uncontact_master   = '20'
      uncontact_secondary    = '10'
      uncontact_displaced = '30'
      blocks              = '1 2'
      include_all_contact_nodes = 1

      petsc_options_iname = '-ksp_type -pc_type -pc_hypre_type '
      petsc_options_value = '  preonly hypre  boomeramg'
    [../]
    [./contact]
      type = ContactSplit
      vars = 'disp_x disp_y leftright_normal_lm leftright_tangential_lm'
      contact_master   = '20'
      contact_secondary    = '10'
      contact_displaced = '30'
      include_all_contact_nodes = 1
      blocks = '4'


      petsc_options_iname = '-ksp_type -pc_sub_type -pc_factor_shift_type  -pc_factor_shift_amount'
      petsc_options_value = '  preonly lu NONZERO 1e-15'
    [../]
  [../]
[]


[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  dt = 0.1
  dtmin = 1e-4
  end_time = 1

  l_tol = 1e-8
  l_max_its = 100

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10
[]

[Outputs]
  file_base = frictional_mortar_FS_out
  [./exodus]
    type = Exodus
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]
