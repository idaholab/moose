
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
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
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 1003
    value = 0.0
  [../]
  [./left_y]
    type = PresetBC
    variable = disp_y
    boundary = 1003
    value = 0.0
  [../]
  [./right_x]
    type = PresetBC
    variable = disp_x
    boundary = 1001
    value = 0.001
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = ORDINARY_STATE
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e8
    poissons_ratio = 0.3
  [../]

  [./force_density]
    type = ComputeSmallStrainConstantHorizonMaterialOSPD
    plane_stress = true
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
  line_search = none
  start_time = 0
  end_time = 1
  nl_rel_tol = 1e-10
[]

[Outputs]
  file_base = conventional_planestress_OSPD
  exodus = true
[]
