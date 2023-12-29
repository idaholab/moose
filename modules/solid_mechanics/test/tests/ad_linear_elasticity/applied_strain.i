[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 2
  ymax = 2
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  eigenstrain_names = eigenstrain
  add_variables = true
  generate_output = 'strain_xx strain_yy strain_xy'
  use_automatic_differentiation = true
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
  [./eigenstrain]
    type = ADComputeEigenstrain
    eigen_base = '0.1 0.05 0 0 0 0.01'
    prefactor = -1
    eigenstrain_name = eigenstrain
  [../]
[]

[BCs]
  [./bottom_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./left_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
