[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  temperature = temp
  out_of_plane_strain = strain_zz
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  [../]
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
    type = StressDivergenceTensors
    variable = disp_x
    eigenstrain_names = thermal_eigenstrain
    component = 0
  [../]
  [./disp_y]
    type = StressDivergenceTensors
    variable = disp_y
    eigenstrain_names = thermal_eigenstrain
    component = 1
  [../]

  [./solid_z]
    type = WeakPlaneStress
    variable = strain_zz
    eigenstrain_names = thermal_eigenstrain
  [../]

  [./heat]
    type = HeatConduction
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
    type = ComputePlaneSmallStrain
    eigenstrain_names = thermal_eigenstrain
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-5
    stress_free_temperature = 0
    eigenstrain_name = thermal_eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]

  [./conductivity]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    use_displaced_mesh = false
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-ksp_type -pc_type -snes_type'
  petsc_options_value = 'bcgs bjacobi test'

  end_time = 1.0
[]
