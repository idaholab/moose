offset = 0.01

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [./left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 0.0
    ymin = -0.5
    ymax = 0.5
    nx = 1
    ny = 1
    elem_type = QUAD4
    boundary_name_prefix = lb
  [../]
  [./left_block_id]
    type = SubdomainIDGenerator
    input = left_block
    subdomain_id = 1
  [../]

  [./right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.0
    xmax = 1.0
    ymin = -0.6
    ymax = 0.6
    nx = 1
    ny = 1
    elem_type = QUAD4
    boundary_name_prefix = rb
    boundary_id_offset = 10
  [../]
  [./right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  [../]

  [./combined]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  [../]
  [./block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'left_block right_block'
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
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
    expression = t/10.0
  [../]
[]

[BCs]
  [./push_x]
    type = FunctionDirichletBC
    preset = true
    variable = disp_x
    boundary = lb_left
    function = horizontal_movement
  [../]
  [./fix_x]
    type = DirichletBC
    preset = true
    variable = disp_x
    boundary = rb_right
    value = 0.0
  [../]
  [./fix_y]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = rb_right
    value = 0.0
  [../]
  [./fix_y_offset]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = lb_left
    value = ${offset}
  [../]
[]

[Materials]
  [./elasticity_tensor_left]
    type = ComputeIsotropicElasticityTensor
    block = left_block
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stress_left]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]

  [./elasticity_tensor_right]
    type = ComputeIsotropicElasticityTensor
    block = right_block
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stress_right]
    type = ComputeFiniteStrainElasticStress
    block = right_block
  [../]
[]

[Contact]
  [./leftright]
    secondary = lb_right
    primary = rb_left

    model = frictionless
    formulation = mortar

    friction_coefficient = 0.0

    normal_smoothing_distance = 0.1

    penalty = 1e+8
    normalize_penalty = true
  [../]
[]

[ICs]
  [./disp_x]
    type = ConstantIC
    block = left_block
    variable = disp_x
    value = -${offset}
  [../]
  [./disp_y]
    block = left_block
    variable = disp_y
    value = ${offset}
    type = ConstantIC
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount -snes_max_it'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15                   20'

  dt = 0.1
  dtmin = 0.1
  end_time = 0.1

  l_tol = 1e-4
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  nl_max_its = 100
[]
