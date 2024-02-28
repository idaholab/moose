# Thermal eigenstrain coupling
[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Variables]
  [./disp_r]
  [../]
  [./disp_z]
  [../]
  [./temperature]
  [../]
[]

[Kernels]
  [./cx_elastic]
    type = StressDivergenceRZTensors
    variable = disp_r
    temperature = temperature
    eigenstrain_names = thermal_contribution
    use_displaced_mesh = false
    component = 0
  [../]
  [./cz_elastic]
    type = StressDivergenceRZTensors
    variable = disp_z
    temperature = temperature
    eigenstrain_names = thermal_contribution
    use_displaced_mesh = false
    component = 1
  [../]
  [./temperature]
    type = Diffusion
    variable = temperature
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10.0
    poissons_ratio = 0.25
  [../]
  [./strain]
    type = ComputeAxisymmetricRZSmallStrain
    eigenstrain_names = thermal_contribution
  [../]
  [./thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1.0E2
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 0.0
  [../]
  [./admissible]
    type = ComputeLinearElasticStress
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
  solve_type = NEWTON
  end_time = 1
  dt = 1
  type = Transient
[]
