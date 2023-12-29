[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 50
  ymax = 50
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./diffused]
     [./InitialCondition]
      type = RandomIC
     [../]
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  use_automatic_differentiation = true
[]

[Kernels]
  [./diff]
    type = ADDiffusion
    variable = diffused
  [../]
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
[]

[BCs]
  [./bottom]
    type = ADDirichletBC
    variable = diffused
    boundary = 'right'
    value = 1
  [../]
  [./top]
    type = ADDirichletBC
    variable = diffused
    boundary = 'top'
    value = 0
  [../]
  [./disp_x_BC]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'bottom top'
    value = 0.5
  [../]
  [./disp_x_BC2]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.01
  [../]
  [./disp_y_BC]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'bottom top'
    value = 0.8
  [../]
  [./disp_y_BC2]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'left right'
    value = 0.02
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
