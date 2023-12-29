# This test involves only thermal expansion strains on a 2x2x2 cube of approximate
# steel material; however, in this case the stress free temperature of the material
# has been set to 200K so that there is an initial delta temperature of 100K.

# An initial temperature of 300K is given for the material,
# and an auxkernel is used to calculate the temperature in the entire cube to
# raise the temperature each time step.  The final temperature is 675K
# The thermal strain increment should therefore be
#     (675K - 300K) * 1.3e-5 1/K + 100K * 1.3e-5 1/K = 6.175e-3 m/m.

# This test uses a start up step to identify problems in the calculation of
# eigenstrains with a stress free temperature that is different from the initial
# value of the temperature in the problem

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [./temp]
    initial_condition = 300.0
  [../]
  [./eigenstrain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./eigenstrain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./eigenstrain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./total_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./total_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./total_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./temperature_load]
    type = ParsedFunction
    expression = t*(5000.0)+300.0
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = SMALL
        incremental = true
        add_variables = true
        eigenstrain_names = eigenstrain
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = temperature_load
  [../]
  [./eigenstrain_yy]
    type = RankTwoAux
    rank_two_tensor = eigenstrain
    variable = eigenstrain_yy
    index_i = 1
    index_j = 1
    execute_on = 'initial timestep_end'
  [../]
  [./eigenstrain_xx]
    type = RankTwoAux
    rank_two_tensor = eigenstrain
    variable = eigenstrain_xx
    index_i = 0
    index_j = 0
    execute_on = 'initial timestep_end'
  [../]
  [./eigenstrain_zz]
    type = RankTwoAux
    rank_two_tensor = eigenstrain
    variable = eigenstrain_zz
    index_i = 2
    index_j = 2
    execute_on = 'initial timestep_end'
  [../]
  [./total_strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = total_strain_yy
    index_i = 1
    index_j = 1
    execute_on = 'initial timestep_end'
  [../]
  [./total_strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = total_strain_xx
    index_i = 0
    index_j = 0
    execute_on = 'initial timestep_end'
  [../]
  [./total_strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = total_strain_zz
    index_i = 2
    index_j = 2
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./small_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 200
    thermal_expansion_coeff = 1.3e-5
    temperature = temp
    eigenstrain_name = eigenstrain
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = -0.0125
  n_startup_steps = 1
  end_time = 0.075
  dt = 0.0125
  dtmin = 0.0001
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [./eigenstrain_xx]
    type = ElementAverageValue
    variable = eigenstrain_xx
    execute_on = 'initial timestep_end'
  [../]
  [./eigenstrain_yy]
    type = ElementAverageValue
    variable = eigenstrain_yy
    execute_on = 'initial timestep_end'
  [../]
  [./eigenstrain_zz]
    type = ElementAverageValue
    variable = eigenstrain_zz
    execute_on = 'initial timestep_end'
  [../]
  [./total_strain_xx]
    type = ElementAverageValue
    variable = total_strain_xx
    execute_on = 'initial timestep_end'
  [../]
  [./total_strain_yy]
    type = ElementAverageValue
    variable = total_strain_yy
    execute_on = 'initial timestep_end'
  [../]
  [./total_strain_zz]
    type = ElementAverageValue
    variable = total_strain_zz
    execute_on = 'initial timestep_end'
  [../]
  [./temperature]
    type = AverageNodalVariableValue
    variable = temp
    execute_on = 'initial timestep_end'
  [../]
[]
