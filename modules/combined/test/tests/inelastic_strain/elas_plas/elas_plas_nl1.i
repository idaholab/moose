#
# Test for effective strain calculation.
# Boundary conditions from NAFEMS test NL1
#
# This is not a verification test. The boundary conditions are applied such
# that the first step generates only elastic stresses. The second and third
# steps generate plastic deformation and the effective strain should be
# increasing throughout the run.
#
[GlobalParams]
  order = FIRST
  family = LAGRANGE
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = one_elem2.e
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
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./pressure]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./eff_plastic_strain]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
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
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  [../]
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = VonMisesStress
    execute_on = timestep_end
  [../]
  [./pressure]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = pressure
    scalar_type = Hydrostatic
    execute_on = timestep_end
  [../]
  [./elastic_strain_xx]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./elastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./elastic_strain_zz]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./plastic_strain_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./plastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./plastic_strain_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./tot_strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = tot_strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./tot_strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = tot_strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./tot_strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = tot_strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./eff_plastic_strain]
    type = MaterialRealAux
    property = effective_plastic_strain
    variable = eff_plastic_strain
  [../]
[]

[Functions]
  [./appl_dispy]
    type = PiecewiseLinear
    x = '0     1.0     2.0     3.0'
    y = '0.0 0.208e-4 0.50e-4 1.00e-4'
  [../]
[]

[BCs]
  [./side_x]
    type = DirichletBC
    variable = disp_x
    boundary = 101
    value = 0.0
  [../]
  [./origin_x]
    type = DirichletBC
    variable = disp_x
    boundary = 103
    value = 0.0
  [../]
  [./bot_y]
    type = DirichletBC
    variable = disp_y
    boundary = 102
    value = 0.0
  [../]
  [./origin_y]
    type = DirichletBC
    variable = disp_y
    boundary = 103
    value = 0.0
  [../]
  [./top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = appl_dispy
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 250e9
    poissons_ratio = 0.25
  [../]
  [./strain]
    type = ComputePlaneFiniteStrain
    block = 1
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'isoplas'
    block = 1
  [../]
  [./isoplas]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 5e6
    hardening_constant = 0.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-4
  l_max_its = 100
  nl_max_its = 20

  dt = 1.0
  start_time = 0.0
  num_steps = 100
  end_time = 3.0
[] # Executioner

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  [../]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./stress_xy]
    type = ElementAverageValue
    variable = stress_xy
  [../]
  [./vonmises]
    type = ElementAverageValue
    variable = vonmises
  [../]
  [./pressure]
    type = ElementAverageValue
    variable = pressure
  [../]
  [./el_strain_xx]
    type = ElementAverageValue
    variable = elastic_strain_xx
  [../]
  [./el_strain_yy]
    type = ElementAverageValue
    variable = elastic_strain_yy
  [../]
  [./el_strain_zz]
    type = ElementAverageValue
    variable = elastic_strain_zz
  [../]
  [./pl_strain_xx]
    type = ElementAverageValue
    variable = plastic_strain_xx
  [../]
  [./pl_strain_yy]
    type = ElementAverageValue
    variable = plastic_strain_yy
  [../]
  [./pl_strain_zz]
    type = ElementAverageValue
    variable = plastic_strain_zz
  [../]
  [./eff_plastic_strain]
    type = ElementAverageValue
    variable = eff_plastic_strain
  [../]
  [./tot_strain_xx]
    type = ElementAverageValue
    variable = tot_strain_xx
  [../]
  [./tot_strain_yy]
    type = ElementAverageValue
    variable = tot_strain_yy
  [../]
  [./tot_strain_zz]
    type = ElementAverageValue
    variable = tot_strain_zz
  [../]
  [./disp_x1]
    type = NodalVariableValue
    nodeid = 0
    variable = disp_x
  [../]
  [./disp_x4]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_x
  [../]
  [./disp_y1]
    type = NodalVariableValue
    nodeid = 0
    variable = disp_y
  [../]
  [./disp_y4]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_y
  [../]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[] # Outputs
