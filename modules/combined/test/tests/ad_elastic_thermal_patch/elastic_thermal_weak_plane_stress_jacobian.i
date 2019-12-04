[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  temperature = temp
  out_of_plane_strain = strain_zz
[]

[Mesh]
  file = 'gold/square.e'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./strain_zz]
  [../]

  [./temp]
  [../]
[]

[Kernels]
  [./disp_x]
    type = ADStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./disp_y]
    type = ADStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]

  [./solid_z]
    type = ADWeakPlaneStress
    variable = strain_zz
  [../]

  [./heat]
    type = ADHeatConduction
    variable = temp
    use_displaced_mesh = false
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.0
    youngs_modulus = 1
  [../]
  [./strain]
    type = ADComputePlaneSmallStrain
    eigenstrain_names = thermal_eigenstrain
  [../]
  [./thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-5
    stress_free_temperature = 0
    eigenstrain_name = thermal_eigenstrain
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]

  [./conductivity]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    use_displaced_mesh = false
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-ksp_type -pc_type -snes_type'
  petsc_options_value = 'bcgs bjacobi test'

  end_time = 1.0
[]
