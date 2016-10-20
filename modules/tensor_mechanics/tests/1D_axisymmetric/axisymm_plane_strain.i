#
# This test checks elastic stress calculations with mechanical and thermal
# strain.   Young's modulus is 3600, and Poisson's ratio is 0.2.
# The axisymmetric, plane strain 1D mesh is pulled with 1e-6 strain.  Thus,
# the strain is [1e-6, 0, 1e-6] (xx, yy, zz).  This give stress of
# [5e-3, 2e-3, 5e-3].  After a temperature increase of 100 with alpha of
# 1e-8, the stress becomes [-1e-3, -4e-3, -1e-3].
#

[GlobalParams]
  # Set initial fuel density, other global parameters
  order = SECOND
  family = LAGRANGE
  displacements = disp_x
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = axisymm_plane_strain.e
[]

[Variables]
  # Define dependent variables and initial conditions
  [./disp_x]
  [../]
[]

[AuxVariables]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
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
    initial_condition = 580.0     # set initial temp to coolant inlet
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
    y = '0 2e-6'
  [../]
[]

[Kernels]
  [./rz]
    type = StressDivergenceRZTensors
    variable = disp_x
    component = 0
  [../]
[]

[AuxKernels]
  # Define auxilliary kernels for each of the aux variables
  [./stress_xx]               # computes stress components for output
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
    boundary = 12
    value = 0
    variable = disp_x
  [../]
  [./disp_x]
    type = FunctionPresetBC
    boundary = 10
    function = disp_x
    variable = disp_x
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = pellet_type_1
    youngs_modulus = 3600
    poissons_ratio = 0.2
  [../]

  [./strain]
    type = ComputeAxisymmetricRZIncrementalPlaneStrain
    block = pellet_type_1
    eigenstrain_names = eigenstrain
  [../]

  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    block = pellet_type_1
    thermal_expansion_coeff = 1e-8
    temperature = temp
    incremental_form = true
    stress_free_temperature = 580
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = pellet_type_1
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  line_search = 'none'

  l_max_its = 50
  l_tol = 8e-3
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  start_time = 0
  end_time = 2
  num_steps = 2
[]

[Outputs]
  # Define output file(s)
  exodus = true
  console = true
[]
