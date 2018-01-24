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
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[UserObjects]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 10
  [../]
  [./phi]
    type = TensorMechanicsHardeningConstant
    value = 0.8
  [../]
  [./psi]
    type = TensorMechanicsHardeningConstant
    value = 0.4
  [../]
  [./dp]
    type = TensorMechanicsPlasticDruckerPragerHyperbolic
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
    C_ijkl = '0.7 1'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '10 0 0  0 10 0  0 0 10'
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
