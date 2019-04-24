# Test for Jacobian correctness check of non-ordinary state-based peridynamic formulation for mechanics problem

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  full_jacobian = true
[]

[MeshGenerators]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
  []
  [gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  []
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 2
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = NonOrdinaryState
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.0
  [../]
  [./strain]
    type = SmallStrainNOSPD
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_type'
    petsc_options_value = 'bcgs bjacobi test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1
  dt = 1
  num_steps = 1
[]
