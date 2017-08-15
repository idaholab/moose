#
# This test checks the generalized plane strain using finite strain formulation.
# since we constrain all the nodes against movement and the applied thermal strain
# is very small, the results are the same as small and incremental small strain formulations
#

[GlobalParams]
  displacements = disp_x
  scalar_out_of_plane_strain = scalar_strain_yy
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = lines.e
[]

[Variables]
  [./disp_x]
  [../]
  [./temp]
    initial_condition = 580.0
  [../]
  [./scalar_strain_yy]
    order = FIRST
    family = SCALAR
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
[]

[Functions]
  [./temp100]
    type = PiecewiseLinear
    x = '0   1'
    y = '580 680'
  [../]
  [./temp300]
    type = PiecewiseLinear
    x = '0   1'
    y = '580 880'
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./GeneralizedPlaneStrain]
      [./gps]
      [../]
    [../]
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
[]

[BCs]
  [./no_x]
    type = PresetBC
    boundary = 1000
    value = 0
    variable = disp_x
  [../]
  [./temp100]
    type = FunctionDirichletBC
    variable = temp
    function = temp100
    boundary = 2
  [../]
  [./temp300]
    type = FunctionDirichletBC
    variable = temp
    function = temp300
    boundary = 3
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
    thermal_expansion_coeff = 1e-8
    temperature = temp
    incremental_form = true
    stress_free_temperature = 580
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]

  [./thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
    specific_heat = 1.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  l_max_its = 50
  l_tol = 1e-08
  nl_max_its = 15
  nl_abs_tol = 1e-10
  start_time = 0
  end_time = 1
  num_steps = 1
[]

[Outputs]
  exodus = true
  console = true
[]
