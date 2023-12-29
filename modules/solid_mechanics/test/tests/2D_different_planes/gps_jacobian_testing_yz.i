[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = square_yz_plane.e
[]

[Variables]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./scalar_strain_xx]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./generalized_plane_strain]
    block = 1
    strain = SMALL
    scalar_out_of_plane_strain = scalar_strain_xx
    out_of_plane_direction = x
    planar_formulation = GENERALIZED_PLANE_STRAIN
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.0
    youngs_modulus = 1
  [../]
  [./stress]
    type = ComputeLinearElasticStress
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
