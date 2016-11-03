#
# This test checks the out of plane stress and constraint.  The model consists
# of two sets of line elements.  One undergoes a temperature rise of 100 with
# the other seeing a temperature rise of 300.  Young's modulus is 3600, and
# Poisson's ratio is 0.2.  The thermal expansion coefficient is 1e-8.  All
# nodes are constrained against movement.
#
# Without the strain_yy constraint, the stress solution would be
# [-6e-3, -6e-3, -6e-3] and [-18e-3, -18e-3, -18e-3] (xx, yy, zz).  The out
# of plane stress kernel works to minimize the out of plane stress, and the
# constraint forces the out of plane strain to be equal.
#
# With an out of plane strain of 3e-3, the stress solution becomes
# [-3e-3, 6e-3, -3e-3] and [-15e-3, -6e-3, -15e-3] (xx, yy, zz).  This gives
# an average out of plane stress of zero.
#

[GlobalParams]
  # Set initial fuel density, other global parameters
  order = FIRST
  family = LAGRANGE
  displacements = disp_x
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = constraint.e
[]

[Variables]
  # Define dependent variables and initial conditions
  [./disp_x]
  [../]
  [./strain_yy]
  [../]
  [./temp]
    initial_condition = 580.0
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
  [./rz]
    type = StressDivergenceRZTensors
    variable = disp_x
    component = 0
  [../]
  [./solid_z]
    type = WeakPlaneStress
    variable = strain_yy
    direction = y
  [../]
  [./heat]
    type = HeatConduction
    variable = temp
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

[Constraints]
  [./syy_20]
    type = EqualValueBoundaryConstraint
    variable = strain_yy
    slave = 1000
    penalty = 1e12
    formulation = kinematic
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 3600
    poissons_ratio = 0.2
  [../]

  [./strain]
    type = ComputeAxisymmetricRZIncrementalPlaneStrain
    block = 1
    strain_yy = strain_yy
  [../]

  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    block = 1
    thermal_expansion_coeff = 1e-8
    temperature = temp
    incremental_form = true
    stress_free_temperature = 580
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]

  [./thermal]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 1.0
    specific_heat = 1.0
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
  nl_abs_tol = 1e-8 #1e-10
  start_time = 0
  end_time = 1
  num_steps = 1
[]

[Outputs]
  # Define output file(s)
  exodus = true
  console = true
[]
