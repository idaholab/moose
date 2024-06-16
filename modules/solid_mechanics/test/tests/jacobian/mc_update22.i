# MC update version, with only MohrCoulomb, cohesion=10, friction angle = 60, psi = 5, smoothing_tol = 1
# Lame lambda = 0.5.  Lame mu = 1

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
  [./ts]
    type = SolidMechanicsHardeningConstant
    value = 1E6
  [../]
  [./cs]
    type = SolidMechanicsHardeningConstant
    value = 1E6
  [../]
  [./coh]
    type = SolidMechanicsHardeningConstant
    value = 10
  [../]
  [./phi]
    type = SolidMechanicsHardeningConstant
    value = 60
    convert_to_radians = true
  [../]
  [./psi]
    type = SolidMechanicsHardeningConstant
    value = 5
    convert_to_radians = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    lambda = 0.5
    shear_modulus = 1.0
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
  [./cmc]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = phi
    dilation_angle = psi
    smoothing_tol = 1
    yield_function_tol = 1.0E-12
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = cmc
    perform_finite_strain_rotations = false
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]
