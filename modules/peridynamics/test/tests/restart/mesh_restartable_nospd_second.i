
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  restart_file_base = mesh_restartable_nospd_out_cp/LATEST
[]

[Mesh]
  file = mesh_restartable_nospd_out_cp/LATEST
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
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  file_base = mesh_restartable_nospd_second_out
  exodus = true
[]
