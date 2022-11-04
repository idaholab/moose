offset = 0.0202

vy = 0.15
vx = 0.040

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
    scaling = 1e-6
  [../]
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    expression = 'if(t<1.0,${vx}*t-${offset},${vx}-${offset})'
  [../]
  [./vertical_movement]
    type = ParsedFunction
    expression = 'if(t<1.0,${offset},${vy}*(t-1.0)+${offset})'
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
    secondary = 10
    primary = 20

    model = coulomb
    formulation = mortar

    friction_coefficient = 0.2
    c_tangential = 1e3
    normal_lm_scaling = 1e-3
    tangential_lm_scaling = 1e-3
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
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor -snes_ksp_ew -pc_svd_monitor'

  petsc_options_iname = '-pc_type -mat_mffd_err'
  petsc_options_value = 'svd      1e-5'

  dt = 0.1
  dtmin = 0.1
  num_steps = 7
  end_time = 4
  line_search = none
  snesmf_reuse_base = false
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [./exodus]
    type = Exodus
  [../]
[]
