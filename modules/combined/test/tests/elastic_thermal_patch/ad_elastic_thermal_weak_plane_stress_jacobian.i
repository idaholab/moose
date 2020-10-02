[GlobalParams]
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

[Modules/TensorMechanics/Master]
  [./plane_stress]
    planar_formulation = WEAK_PLANE_STRESS
    strain = SMALL
    eigenstrain_names = thermal_eigenstrain
    use_automatic_differentiation = true
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = false
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.0
    youngs_modulus = 1
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
