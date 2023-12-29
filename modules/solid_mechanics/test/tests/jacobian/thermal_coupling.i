# Thermal eigenstrain coupling
[Mesh]
  type = GeneratedMesh
  dim = 3
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
  [./temperature]
  [../]
[]

[Kernels]
  [./cx_elastic]
    type = StressDivergenceTensors
    variable = disp_x
    temperature = temperature
    eigenstrain_names = thermal_contribution
    component = 0
  [../]
  [./cy_elastic]
    type = StressDivergenceTensors
    variable = disp_y
    temperature = temperature
    eigenstrain_names = thermal_contribution
    component = 1
  [../]
  [./cz_elastic]
    type = StressDivergenceTensors
    variable = disp_z
    temperature = temperature
    eigenstrain_names = thermal_contribution
    component = 2
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
    type = ComputeSmallStrain
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
