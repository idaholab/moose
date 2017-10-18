# Tensile, update version, with strength = 1MPa and smoothing_tol = 0.1E5
# Lame lambda = 1GPa.  Lame mu = 1.3GPa
# Units in this file are MPa (not Pa)
#
# Start from non-diagonal stress state with softening.
# Returns to the plane of tensile yield

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
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningCubic
    value_0 = 1
    value_residual = 0.5
    internal_limit = 2E-2
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    lambda = 0.5E3
    shear_modulus = 1.0E3
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '-1 0.1 0.2  0.1 15 -0.3  0.2 -0.3 0'
    eigenstrain_name = ini_stress
  [../]
  [./tensile]
    type = TensileStressUpdate
    tensile_strength = ts
    smoothing_tol = 0.1
    yield_function_tol = 1.0E-12
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = tensile
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
