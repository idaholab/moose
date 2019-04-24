
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshGenerators]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  []
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[BCs]
  [./left_dx]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./left_dy]
    type = PresetBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./right_dx]
    type = PresetBC
    variable = disp_x
    boundary = right
    value = 0.001
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
    youngs_modulus = 2.1e8
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = PlaneSmallStrainNOSPD
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
  solve_type = PJFNK
  start_time = 0
  end_time = 1
  # num_steps = 2
[]

[Outputs]
  file_base = mesh_restartable_nospd_out
  exodus = true
  checkpoint = true
[]
