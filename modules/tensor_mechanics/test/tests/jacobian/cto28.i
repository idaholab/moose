#Cosserat capped weak plane and capped drucker prager
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
  [./cx_elastic]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./cy_elastic]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./cz_elastic]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 10
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 10
  [../]
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
    yield_function_tolerance = 1E-11     # irrelevant here
    internal_constraint_tolerance = 1E-9 # irrelevant here
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10.0
    poissons_ratio = 0.25
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '10 0 0  0 10 0  0 0 10'
    eigenstrain_name = ini_stress
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'dp'
    relative_tolerance = 2.0
    absolute_tolerance = 1E6
    max_iterations = 1
  [../]
  [./dp]
    type = CappedDruckerPragerStressUpdate
    base_name = dp
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-11
    tip_smoother = 1
    smoothing_tol = 1
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
  solve_type = 'NEWTON'
  end_time = 1
  dt = 1
  type = Transient
[]
