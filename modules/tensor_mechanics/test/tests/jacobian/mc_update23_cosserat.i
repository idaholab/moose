# Cosserat version of Capped Mohr Columb (using StressUpdate)
# Tensile + shear failure, starting from a symmetric stress state

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
    value = 1
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1E6
  [../]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 4E1
  [../]
  [./phi]
    type = TensorMechanicsHardeningConstant
    value = 35
    convert_to_radians = true
  [../]
  [./psi]
    type = TensorMechanicsHardeningConstant
    value = 5
    convert_to_radians = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 1
    poisson = 0.25
    layer_thickness = 1.0
    joint_normal_stiffness = 2.0
    joint_shear_stiffness = 1.0
  [../]
  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '10 12 -14.9  12 5 20  -14 20 8'
    eigenstrain_name = ini_stress
  [../]
  [./cmc]
    type = CappedMohrCoulombCosseratStressUpdate
    host_youngs_modulus = 1
    host_poissons_ratio = 0.25
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = phi
    dilation_angle = psi
    smoothing_tol = 0.5
    yield_function_tol = 1.0E-12
  [../]
  [./stress]
    type = ComputeMultipleInelasticCosseratStress
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
