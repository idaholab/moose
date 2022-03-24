# Hertz Contact: Sphere on sphere

# Spheres have the same radius, Young's modulus, and Poisson's ratio.

# Define E:
# 1/E = (1-nu1^2)/E1 + (1-nu2^2)/E2
#
# Effective radius R:
# 1/R = 1/R1 + 1/R2
#
# F is the applied compressive load.
#
# Area of contact a::
# a^3 = 3FR/4E
#
# Depth of indentation d:
# d = a^2/R
#
#
# Let R1 = R2 = 2.  Then R = 1.
#
# Let nu1 = nu2 = 0.25, E1 = E2 = 1.40625e7.  Then E = 7.5e6.
#
# Let F = 10000.  Then a = 0.1, d = 0.01.
#

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y disp_z'
  order = SECOND
[]

[Mesh]#Comment
  file = hertz_contact_hex20.e
  allow_renumbering = false
[] # Mesh

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 795.77471545947674 # 10000/pi/2^2
  [../]
  [./disp_y]
    type = PiecewiseLinear
    x = '0.  1.    2.'
    y = '0. -0.01 -0.01'
  [../]
[] # Functions

[Variables]

  [./disp_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./disp_y]
    order = SECOND
    family = LAGRANGE
  [../]

  [./disp_z]
    order = SECOND
    family = LAGRANGE
  [../]

[] # Variables

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
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydrostatic]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]

[] # AuxVariables

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = SMALL
    extra_vector_tags = 'ref'
    save_in = 'saved_x saved_y saved_z'
  [../]
[]
[AuxKernels]

  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = stress_xx
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = stress_yy
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_zz
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = stress_xy
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = stress_yz
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 0
    variable = stress_zx
  [../]
#  [./vonmises]
#    type = RankTwoScalarAux
#    rank_two_tensor = stress
#    variable = vonmises
#    scalar_type = VonMisesStress
#  [../]

[] # AuxKernels

[BCs]

  [./base_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1000
    value = 0.0
  [../]
  [./base_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1000
    value = 0.0
  [../]
  [./base_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1000
    value = 0.0
  [../]

  [./symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./symm_z]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]
  [./disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = disp_y
  [../]

[] # BCs

[Contact]
  [./dummy_name]
    primary = 1000
    secondary = 100

    normalize_penalty = true
    tangential_tolerance = 1e-3
    penalty = 1e+10
  [../]
[]

#[Dampers]
#  [./contact_slip]
#    type = ContactSlipDamper
#    primary = 1000
#    secondary = 100
#  [../]
#[]

[Materials]
  [./tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1'
  [../]

  [./tensor_1000]
    type = ComputeIsotropicElasticityTensor
    block = '1000'
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stress_1000]
    type = ComputeLinearElasticStress
    block = '1000'
  [../]

[] # Materials

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]

  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'


  nl_abs_tol = 1e-7

  l_max_its = 10

  start_time = 0.0
  dt = 0.05
  end_time = 2.0

  [./Quadrature]
    order = THIRD
  [../]

[] # Executioner

[Postprocessors]
  [./maxdisp]
    type = NodalVariableValue
    nodeid = 386 # 387-1 where 387 is the exodus node number of the top-center node
    variable = disp_y
  [../]
  [./bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 2
  [../]
  [./bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 2
  [../]

  [./bot_react_z]
    type = NodalSum
    variable = saved_z
    boundary = 2
  [../]

[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
