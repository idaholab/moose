# CappedDruckerPragerCosserat

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
  displacements = 'disp_x disp_y disp_z'
  Cosserat_rotations = 'wc_x wc_y wc_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./wc_x]
  [../]
  [./wc_y]
  [../]
[]


[Kernels]
  [./cx_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./cy_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./cz_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./x_couple]
    type = StressDivergenceTensors
    variable = wc_x
    displacements = 'wc_x wc_y wc_z'
    component = 0
    base_name = couple
  [../]
  [./y_couple]
    type = StressDivergenceTensors
    variable = wc_y
    displacements = 'wc_x wc_y wc_z'
    component = 1
    base_name = couple
  [../]
  [./x_moment]
    type = MomentBalancing
    variable = wc_x
    component = 0
  [../]
  [./y_moment]
    type = MomentBalancing
    variable = wc_y
    component = 1
  [../]
[]

[AuxVariables]
  [./wc_z]
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
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 2.1
    poisson = 0.1
    layer_thickness = 1.0
    joint_normal_stiffness = 3.0
    joint_shear_stiffness = 2.5
  [../]
  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '6 5 4  5.1 7 2  4 2.1 2'
    eigenstrain_name = ini_stress
  [../]
  [./admissible]
    type = ComputeMultipleInelasticCosseratStress
    inelastic_models = dp
  [../]
  [./dp]
    type = CappedDruckerPragerCosseratStressUpdate
    host_youngs_modulus = 2.1
    host_poissons_ratio = 0.1
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-11
    tip_smoother = 0.1
    smoothing_tol = 0.1
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
