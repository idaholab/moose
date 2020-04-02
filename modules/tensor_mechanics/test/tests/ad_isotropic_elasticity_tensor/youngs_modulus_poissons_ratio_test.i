[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./stress_11]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    use_automatic_differentiation = true
  [../]
[]

[AuxKernels]
  [./stress_11]
    type = ADRankTwoAux
    variable = stress_11
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
  [../]
[]

[BCs]
  [./bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./left]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./back]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./top]
    type = ADDirichletBC
    variable = disp_y
    boundary = top
    value = 0.001
  [../]
[]

[Materials]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.1
    youngs_modulus = 1e6
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
  l_max_its = 20
  nl_max_its = 10
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
