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
#    ux = 1e-4*(2x + y + z)/2
#    uy = 1e-4*(x + 2y + z)/2
#    ux = 1e-4*(x + y + 2z)/2
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
#    stress xy = 2 * 4e5 * 1e-4/2 = 40
#

[Mesh]

  file = elastic_patch_quadratic.e
  displacements = 'disp_x disp_y disp_z'

[] # Mesh

[Functions]

  [./xDispFunc]
    type = ParsedFunction
    value = 5e-5*(2*x+y+z)
  [../]
  [./yDispFunc]
    type = ParsedFunction
    value = 5e-5*(x+2*y+z)
  [../]
  [./zDispFunc]
    type = ParsedFunction
    value = 5e-5*(x+y+2*z)
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

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]

  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]
  [./elastic_energy]
    type = ElasticEnergyAux
    variable = elastic_energy
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
  [../]
  [./hydrostatic]
    type = MaterialTensorAux
    tensor = stress
    variable = hydrostatic
    quantity = hydrostatic
  [../]
  [./fi]
    type = MaterialTensorAux
    tensor = stress
    variable = firstinv
    quantity = firstinvariant
  [../]
  [./si]
    type = MaterialTensorAux
    tensor = stress
    variable = secondinv
    quantity = secondinvariant
  [../]
  [./ti]
    type = MaterialTensorAux
    tensor = stress
    variable = thirdinv
    quantity = thirdinvariant
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

  [./stiffStuff1]
    type = SolidModel
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.25
    constitutive_model = elastic
  [../]
  [./elastic]
    type = ElasticModel
    block = 1
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
  output_initial = true
  print_linear_residuals = true
  print_perf_log = true
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
