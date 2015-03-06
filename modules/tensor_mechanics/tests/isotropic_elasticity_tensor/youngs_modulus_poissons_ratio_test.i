[Mesh]
  type = GeneratedMesh
  dim = 3
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

[Kernels]
  [./TensorMechanics]
    disp_z = disp_z
    disp_y = disp_y
    disp_x = disp_x
  [../]
[]

[AuxKernels]
  [./stress_11]
    type = RankTwoAux
    variable = stress_11
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
  [../]
[]

[BCs]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./back]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./top]
    type = PresetBC
    variable = disp_y
    boundary = top
    value = 0.001
  [../]
[]

[Materials]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    disp_z = disp_z
    disp_y = disp_y
    disp_x = disp_x
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
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
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]

