[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = FileMesh
  file = circ_emb_level4.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[BCs]
  [./2_x]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  [../]
  [./2_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./2_z]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0.7E7 1E7'
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./density]
    type = Density
    density = 8050.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-4

  l_max_its = 3

  start_time = 0.0
  dt = 0.1
  num_steps = 1
  end_time = 1.0
[]

[Postprocessors]
  [./time_step]
    type = CriticalTimeStep
  [../]
[]

[Outputs]
  file_base = out
  exodus = true
[]
