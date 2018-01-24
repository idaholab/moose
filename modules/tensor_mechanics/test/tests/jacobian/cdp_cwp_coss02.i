#Cosserat capped weak plane and capped drucker prager, coming back to a mix of shear and tensile failure in both
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

  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 2
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 2.055555555556E-01
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 100
  [../]

[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 10.0
    poisson = 0.25
    layer_thickness = 10.0
    joint_normal_stiffness = 2.5
    joint_shear_stiffness = 2.0
  [../]
  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '1 0.1 0  0.1 2 0  11 12 10' # note unsymmetric
    eigenstrain_name = ini_stress
  [../]
  [./admissible]
    type = ComputeMultipleInelasticCosseratStress
    inelastic_models = 'dp wp'
    relative_tolerance = 2.0
    absolute_tolerance = 1E6
    max_iterations = 1
  [../]
  [./dp]
    type = CappedDruckerPragerCosseratStressUpdate
    host_youngs_modulus = 10.0
    host_poissons_ratio = 0.25
    base_name = dp
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-11
    tip_smoother = 1
    smoothing_tol = 1
  [../]
  [./wp]
    type = CappedWeakPlaneCosseratStressUpdate
    base_name = wp
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0.1
    smoothing_tol = 0.1
    yield_function_tol = 1E-11
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
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
