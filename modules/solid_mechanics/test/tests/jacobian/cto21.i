# DruckerPragerHyperbolic

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
  [SolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[UserObjects]
  [./mc_coh]
    type = SolidMechanicsHardeningCubic
    value_0 = 10
    value_residual = 1
    internal_limit = 100
  [../]
  [./phi]
    type = SolidMechanicsHardeningCubic
    value_0 = 0.8
    value_residual = 0.4
    internal_limit = 50
  [../]
  [./psi]
    type = SolidMechanicsHardeningCubic
    value_0 = 0.4
    value_residual = 0
    internal_limit = 10
  [../]
  [./dp]
    type = SolidMechanicsPlasticDruckerPragerHyperbolic
    mc_cohesion = mc_coh
    mc_friction_angle = phi
    mc_dilation_angle = psi
    smoother = 1
    yield_function_tolerance = 1E-11
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 1'
  [../]
  [./strain]
    type = ComputeIncrementalStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '6 5 4  5 7 2  4 2 2'
    eigenstrain_name = ini_stress
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-11
    plastic_models = dp
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
