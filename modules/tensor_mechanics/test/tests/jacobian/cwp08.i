# Capped weak-plane plasticity
# checking jacobian for shear + compression failure
[Mesh]
  type = GeneratedMesh
  dim = 3
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
  [./coh]
    type = TensorMechanicsHardeningExponential
    value_0 = 1
    value_residual = 1
    rate = 1
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningExponential
    value_0 = 1.0
    value_residual = 1.0
    rate = 2
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningExponential
    value_0 = 0.1
    value_residual = 0.1
    rate = 1
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningExponential
    value_0 = 100
    value_residual = 100
    rate = 1
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    lambda = 0.0
    shear_modulus = 2.0
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '0 0 1  0 0 -1  1 -1 0'
    eigenstrain_name = ini_stress
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    tangent_operator = nonlinear
  [../]
  [./mc]
    type = CappedWeakPlaneStressUpdate
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    max_NR_iterations = 20
    tip_smoother = 0
    smoothing_tol = 2
    yield_function_tol = 1E-10
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
