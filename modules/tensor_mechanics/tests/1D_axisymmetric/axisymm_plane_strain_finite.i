#
# This test checks elastic stress calculations with mechanical and thermal
# strain using finite strain formulation. Young's modulus is 3600, and Poisson's ratio is 0.2.
# The axisymmetric, plane strain 1D mesh is pulled with displacement of 2e-3.
# Thus, the strain is [log(1+1e-3)=9.995e-4, 0, log(1+1e-3)=9.995e-4] (xx, yy, zz). This gives stress of
# [4.9975, 1.999, 4.9975].  After a temperature increase of 100 with alpha of
# 1e-6, the stress becomes [4.3975, 1.399, 4.3975].
#

[GlobalParams]
  displacements = disp_x
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = line.e
[]

[Variables]
  [./disp_x]
  [../]
[]

[AuxVariables]
  [./strain_xx]
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
  [./stress_xx]
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
  [./temp]
    initial_condition = 580.0
  [../]
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '580 580 680'
  [../]
  [./disp_x]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 2e-3'
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[AuxKernels]
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = 'timestep_begin'
  [../]
[]

[BCs]
  [./no_x]
    type = PresetBC
    boundary = 1
    value = 0
    variable = disp_x
  [../]
  [./disp_x]
    type = FunctionPresetBC
    boundary = 2
    function = disp_x
    variable = disp_x
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3600
    poissons_ratio = 0.2
  [../]

  [./strain]
    type = ComputeAxisymmetric1DFiniteStrain
    eigenstrain_names = eigenstrain
  [../]

  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-6
    temperature = temp
    incremental_form = true
    stress_free_temperature = 580
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  l_max_its = 50
  l_tol = 1e-6
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  start_time = 0
  end_time = 2
  num_steps = 2
[]

[Outputs]
  exodus = true
  console = true
[]
