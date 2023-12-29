[GlobalParams]
  displacements = 'disp_x disp_y'
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
[]

[Modules/TensorMechanics/Master]
  [./plane_stress]
    planar_formulation = WEAK_PLANE_STRESS
    strain = SMALL
    use_automatic_differentiation = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.0
    youngs_modulus = 1
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-ksp_type -pc_type -snes_type'
  petsc_options_value = 'bcgs bjacobi test'

  end_time = 1.0
[]
