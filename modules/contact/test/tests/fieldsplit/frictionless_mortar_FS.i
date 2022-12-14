offset = 0.021

vy = 0.15
vx = 0.04

refine = 1

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [original_file_mesh]
    type = FileMeshGenerator
    file = long_short_blocks.e
  []
  uniform_refine = ${refine}
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    block = '1 2'
    use_automatic_differentiation = true
  []
[]

[Functions]
  [horizontal_movement]
    type = ParsedFunction
    value = 'if(t<0.5,${vx}*t-${offset},${vx}-${offset})'
  []
  [vertical_movement]
    type = ParsedFunction
    value = 'if(t<0.5,${offset},${vy}*(t-0.5)+${offset})'
  []
[]

[BCs]
  [push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30
    function = horizontal_movement
    preset = false
  []
  [fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = '40'
    value = 0.0
  []
  [push_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '30'
    function = vertical_movement
    preset = false
  []
[]

[Materials]
  [elasticity_tensor_left]
    type = ADComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_left]
    type = ADComputeFiniteStrainElasticStress
    block = 1
  []
  [elasticity_tensor_right]
    type = ADComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_right]
    type = ADComputeFiniteStrainElasticStress
    block = 2
  []
[]

[Contact]
  [leftright]
    secondary = 10
    primary = 20
    model = frictionless
    formulation = mortar
    c_normal = 1e6
  []
[]

[ICs]
  [disp_y]
    block = 1
    variable = disp_y
    value = ${offset}
    type = ConstantIC
  []
  [disp_x]
    block = 1
    variable = disp_x
    value = -${offset}
    type = ConstantIC
  []
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'contact_interior'
    [contact_interior]
      splitting = 'interior contact'
      splitting_type = schur
      petsc_options = '-snes_ksp_ew'
      petsc_options_iname = '-ksp_gmres_restart -pc_fieldsplit_schur_fact_type -mat_mffd_err'
      petsc_options_value = '200                full                           1e-5'
      schur_pre = 'S'
    []
    [interior]
      vars = 'disp_x disp_y'
      petsc_options_iname = '-ksp_type -pc_type -pc_hypre_type '
      petsc_options_value = 'gmres   hypre  boomeramg'
    []
    [contact]
      vars = 'leftright_normal_lm'
    []
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  dt = 0.1
  end_time = 1
  abort_on_solve_fail = true
  l_max_its = 200
  nl_abs_tol = 1e-8
  line_search = 'none'
  nl_max_its = 20
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [lin]
    type = NumLinearIterations
    outputs = 'console'
  []
  [cum]
    type = CumulativeValuePostprocessor
    postprocessor = 'lin'
    outputs = 'console'
  []
[]
