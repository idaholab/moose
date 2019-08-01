
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  restart_file_base = 2D_mesh_restartable_nospd_out_cp/LATEST
[]

[Mesh]
  file = 2D_mesh_restartable_nospd_out_cp/LATEST
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
    formulation = NONORDINARY_STATE
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e8
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputePlaneSmallStrainNOSPD
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

  [./Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  [../]
[]

[Outputs]
  file_base = 2D_mesh_restartable_NOSPD_second_out
  exodus = true
[]
