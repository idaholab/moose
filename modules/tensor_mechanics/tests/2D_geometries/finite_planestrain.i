[Mesh]
  type = GeneratedMesh
  nx = 2
  ny = 2
  dim = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_j = 0
    index_i = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_j = 1
    index_i = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_j = 1
    index_i = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_j = 2
    index_i = 2
  [../]

  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_j = 0
    index_i = 0
  [../]
  [./strain_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xy
    index_j = 1
    index_i = 0
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_j = 1
    index_i = 1
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_j = 2
    index_i = 2
  [../]
[]

[Functions]
  [./pull]
    type = ParsedFunction
    value ='0.005 * t'
  [../]
[]

[BCs]
  [./leftx]
    type = PresetBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./pull]
    type = FunctionPresetBC
    boundary = top
    variable = disp_y
    function = pull
  [../]
[]

[Materials]
  [./small_strain]
    type = ComputePlaneFiniteStrain
    block = 0
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none


# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-10

# controls for nonlinear iterations
  nl_max_its = 10
  nl_rel_tol = 1e-12

# time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
