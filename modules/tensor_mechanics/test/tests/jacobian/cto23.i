# MeanCapTC with compressive failure

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[GlobalParams]
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]


[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[UserObjects]
  [./tensile_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = 10
    value_residual = 1
    internal_limit = 10
  [../]
  [./compressive_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = -10
    value_residual = -1
    internal_limit = 9
  [../]
  [./cap]
    type = TensorMechanicsPlasticMeanCapTC
    tensile_strength = tensile_strength
    compressive_strength = compressive_strength
    yield_function_tolerance = 1E-11
    internal_constraint_tolerance = 1E-9
    use_custom_cto = true
    use_custom_returnMap = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0.7 1'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '-6 5 4  5 -7 2  4 2 -2'
    eigenstrain_name = ini_stress
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-11
    plastic_models = cap
    tangent_operator = nonlinear
    min_stepsize = 1
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]
