# Patch Test for second order hex elements (HEX20)
#
# From Abaqus, Verification Manual, 1.5.2
#
# This test is designed to compute constant xx, yy, zz, xy, yz, and zx
#  stress on a set of irregular hexes.  The mesh is composed of one
#  block with seven elements.  The elements form a unit cube with one
#  internal element.  There is a nodeset for each exterior node.

# The cube is displaced on all exterior nodes using the functions,
#
#    ux = 1e-4 * (2x + y + z) / 2
#    uy = 1e-4 * (x + 2y + z) / 2
#    ux = 1e-4 * (x + y + 2z) / 2
#
#  giving uniform strains of
#
#    exx = eyy = ezz = 2*exy = 2*eyz = 2*exz = 1e-4
#
#
# Hooke's Law provides an analytical solution for the uniform stress state.
#  For example,
#
#    stress xx = lambda(exx + eyy + ezz) + 2 * G * exx
#    stress xy = 2 * G * exy
#
#   where:
#
#    lambda = (2 * G * nu) / (1 - 2 * nu)
#    G = 0.5 * E / (1 + nu)
#
# For the test below, E = 1e6 and nu = 0.25, giving lambda = G = 4e5
#
# Thus
#
#    stress xx = 4e5 * (3e-4) + 2 * 4e5 * 1e-4 = 200
#    stress xy = 2 * 4e5 * 1e-4 / 2 = 40
#
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]
[Mesh]
  file = elastic_patch_quadratic.e
[] # Mesh

[Functions]
  [./xDispFunc]
    type = ParsedFunction
    expression = 5e-5*(2*x+y+z)
  [../]
  [./yDispFunc]
    type = ParsedFunction
    expression = 5e-5*(x+2*y+z)
  [../]
  [./zDispFunc]
    type = ParsedFunction
    expression = 5e-5*(x+y+2*z)
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
  [./elastic_energy]
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
  [./firstinv]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./secondinv]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./thirdinv]
    order = CONSTANT
    family = MONOMIAL
  [../]
[] # AuxVariables

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
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
  [./elastic_energy]
    type = ElasticEnergyAux
    variable = elastic_energy
  [../]
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = VonMisesStress
    variable = vonmises
  [../]
  [./hydrostatic]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = Hydrostatic
    variable = hydrostatic
  [../]
  [./fi]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = FirstInvariant
    variable = firstinv
  [../]
  [./si]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = SecondInvariant
    variable = secondinv
  [../]
  [./ti]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = ThirdInvariant
    variable = thirdinv
  [../]
[] # AuxKernels

[BCs]
  [./all_nodes_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 2 3 4 6 7 8 9 10 12 15 17 18 19 20 21 23 24 25 26'
    function = xDispFunc
  [../]
  [./all_nodes_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '1 2 3 4 6 7 8 9 10 12 15 17 18 19 20 21 23 24 25 26'
    function = yDispFunc
  [../]
  [./all_nodes_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = '1 2 3 4 6 7 8 9 10 12 15 17 18 19 20 21 23 24 25 26'
    function = zDispFunc
  [../]
[] # BCs

[Materials]
  [./elast_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[] # Materials

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-6

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
