[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 15

  xmin = 0
  xmax = 2

  ymin = 0
  ymax = 2

  zmin = 0
  zmax = 1
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
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.1
    youngs_modulus = 1e6
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '8050.0'
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
  csv = true
[]
