#
# This is similar to the patch test for 1D spherical elements with the
#   addition of a contact interface.
#
# The 1D mesh is pinned at x=0.  The displacement at the outer node is set to
#   -3e-3*X where X is the x-coordinate of that node.  That gives a strain of
#   -3e-3 for the x, y, and z directions.
#
# Young's modulus is 1e6, and Poisson's ratio is 0.25.  This gives:
#
# Stress xx, yy, zz = E/(1+nu)/(1-2nu)*strain*((1-nu) + nu + nu) = -6000
#

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]
  file = simple_contact_rspherical.e
  construct_side_list_from_node_list = true
[]

[GlobalParams]
  displacements = 'disp_x'
  volumetric_locking_correction = false
[]

[Functions]
  [./ur]
    type = ParsedFunction
    value = '-3e-3*x'
  [../]
[]

[Variables]
  [./disp_x]
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
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
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
[]

[BCs]
  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 4'
    function = ur
  [../]
[]

[Contact]
  [./fred]
    primary = 2
    secondary = 3
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    formulation = SphericalR
    block = '1 2 3'

    disp_r = disp_x

    youngs_modulus = 1e6
    poissons_ratio = 0.25
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-11

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
