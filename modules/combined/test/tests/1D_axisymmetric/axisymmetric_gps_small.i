#
# This test checks the generalized plane strain using small strain formulation.
# The model consists of two sets of line elements. One undergoes a temperature rise of 100 with
# the other seeing a temperature rise of 300.  Young's modulus is 3600, and
# Poisson's ratio is 0.2.  The thermal expansion coefficient is 1e-8.  All
# nodes are constrained against movement.
#
# For plane strain case, i.e., without constraining the strain_yy to be uniform,
# the stress solution would be [-6e-3, -6e-3, -6e-3] and [-18e-3, -18e-3, -18e-3] (xx, yy, zz).
# The generalized plane strain kernels work to balance the force in y direction.
#
# With out of plane strain of 3e-6, the stress solution becomes
# [-3e-3, 6e-3, -3e-3] and [-15e-3, -6e-3, -15e-3] (xx, yy, zz).  This gives
# a domain integral of out-of-plane stress to be zero.
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
    type = ComputeAxisymmetric1DSmallStrain
    eigenstrain_names = eigenstrain
  [../]

  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-8
    temperature = temp
    stress_free_temperature = 580
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
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
