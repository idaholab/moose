# CappedDruckerPrager and CappedWeakPlane, both with all parameters softening/hardening.
# With large tolerance in ComputeMultipleInelasticStress so that only 1 iteration is performed

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
  [./ts]
    type = TensorMechanicsHardeningCubic
    value_0 = 1
    value_residual = 2
    internal_limit = 100
  [../]
  [./cs]
    type = TensorMechanicsHardeningCubic
    value_0 = 5
    value_residual = 3
    internal_limit = 100
  [../]
  [./mc_coh]
    type = TensorMechanicsHardeningCubic
    value_0 = 10
    value_residual = 1
    internal_limit = 100
  [../]
  [./phi]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.8
    value_residual = 0.4
    internal_limit = 50
  [../]
  [./psi]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.4
    value_residual = 0
    internal_limit = 10
  [../]
  [./dp]
    type = TensorMechanicsPlasticDruckerPragerHyperbolic
    mc_cohesion = mc_coh
    mc_friction_angle = phi
    mc_dilation_angle = psi
    yield_function_tolerance = 1E-11     # irrelevant here
    internal_constraint_tolerance = 1E-9 # irrelevant here
  [../]
  [./wp_ts]
    type = TensorMechanicsHardeningExponential
    value_0 = 100
    value_residual = 100
    rate = 1
  [../]
  [./wp_cs]
    type = TensorMechanicsHardeningCubic
    value_0 = 1
    value_residual = 0
    internal_0 = -2
    internal_limit = 0
  [../]
  [./wp_coh]
    type = TensorMechanicsHardeningExponential
    value_0 = 1
    value_residual = 2
    rate = 1
  [../]
  [./wp_tanphi]
    type = TensorMechanicsHardeningExponential
    value_0 = 1.0
    value_residual = 0.5
    rate = 2
  [../]
  [./wp_tanpsi]
    type = TensorMechanicsHardeningExponential
    value_0 = 0.1
    value_residual = 0.05
    rate = 3
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    lambda = 0.1
    shear_modulus = 1.0
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '6 5 4  5 7 2  4 2 2'
    eigenstrain_name = ini_stress
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'dp wp'
    relative_tolerance = 1E4
    absolute_tolerance = 2
    tangent_operator = nonlinear
  [../]
  [./dp]
    type = CappedDruckerPragerStressUpdate
    base_name = cdp
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-11
    tip_smoother = 1
    smoothing_tol = 1
  [../]
  [./wp]
    type = CappedWeakPlaneStressUpdate
    base_name = cwp
    cohesion = wp_coh
    tan_friction_angle = wp_tanphi
    tan_dilation_angle = wp_tanpsi
    tensile_strength = wp_ts
    compressive_strength = wp_cs
    tip_smoother = 0
    smoothing_tol = 1
    yield_function_tol = 1E-11
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
