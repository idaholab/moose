offset = 0.001
vy = 0.1

refine = 0

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

    use_displaced_mesh = true
  [../]
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    value = '0.002-0.003*exp(-4.05*t)'
  [../]
  [./vertical_movement]
    type = ParsedFunction
    value = '${vy}*t+${offset}'
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
    primary = 20

    use_dual = true

    formulation = mortar
    model = frictionless
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
    value = ${offset}
    type = ConstantIC
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew -snes_fd'

  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO   1e-15'

  dt = 0.2
  dtmin = 1e-4
  end_time = 0.2

  l_max_its = 100

  nl_max_its = 30
  nl_rel_tol = 1e-6
[]

[Outputs]
  file_base = ./dm_contact_out
  [./comp]
    type = CSV
    show = 'contact normal_lm avg_disp_x avg_disp_y max_disp_x max_disp_y min_disp_x min_disp_y'
  [../]
[]


[Postprocessors]
  [./contact]
    type = ContactDOFSetSize
    variable = leftright_normal_lm
    subdomain = leftright_secondary_subdomain
  []
  [./normal_lm]
    type = ElementAverageValue
    variable = leftright_normal_lm
    block = '4'
  [../]
  [./avg_disp_x]
    type = ElementAverageValue
    variable = disp_x
    block = '1 2'
  [../]
  [./avg_disp_y]
    type = ElementAverageValue
    variable = disp_y
    block = '1 2'
  [../]
  [./max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
  [../]
  [./max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
    block = '1 2'
  [../]
  [./min_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
    value_type = min
  [../]
  [./min_disp_y]
    type = ElementExtremeValue
    variable = disp_y
    block = '1 2'
    value_type = min
  [../]
[]
