# This test uses small strain formulation, and the use_displaced_mesh
# is set to false for the kernels
#
# Gravity Test
#
# This test is similar to the other gravity tests, but it also tests the
# capability in MaterialTensorAux to return the stress of a single,
# specified integration point, rather than the element average.
# To get the stress at a single integration point, set the parameter
# qp_select to the integration point number (i.e. 0-9 for a quad 8)
# in the AuxKernel
#
# The mesh for this problem is a unit square.
#
# The boundary conditions for this problem are as follows.  The
#   displacement is zero on each of side that faces a negative
#   coordinate direction.  The acceleration of gravity is 20.
#
# The material has a Young's modulus of 1e6 and a density of 2.
#
# The analytic solution for the displacement along the bar is:
#
# u(x) = -b*x^2/(2*E)+b*L*x/E
#
# The displacement at x=L is b*L^2/(2*E) = 2*20*1*1/(2*1e6) = 0.00002.
#
# The analytic solution for the stress along the bar assuming linear
#   elasticity is:
#
# S(x) = b*(L-x)
#
# The stress at x=0 is b*L = 2*20*1 = 40.
#
# Note:  The isoparametric coordinate for a quad8 (fourth order) element
# is: +/- 0.77459667 and 0.  For a 1 unit square with the edge of
# the element in the x = 0 plane, there would be an integration point
# at x_coordinate 0.5 - 0.5*0.77459667 (0.11270167), 0.5, and
# 0.50 + 0.5*0.77459667 (0.88729834).
#
# The corresponding stresses are:
#
# S(0.11270167) = 40(1-0.11270167) = 35.491933
# S(0.5) = 40(1-0.5) = 20
# S(0.88729834) = 40(1-0.88729834) = 4.5080664
#
# These stresses are a precise match to the simulation result.
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = gravity_2D.e
[]

[Variables]
  [./disp_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./disp_y]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_5]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_6]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_7]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_qp_8]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[Kernels]
  [./gravity]
    type = Gravity
    variable = disp_x
    value = 20
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_xx_qp_0]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_0
    index = 0
    qp_select = 0
  [../]
  [./stress_xx_qp_1]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_1
    index = 0
    qp_select = 1
  [../]
  [./stress_xx_qp_2]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_2
    index = 0
    qp_select = 2
  [../]
  [./stress_xx_qp_3]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_3
    index = 0
    qp_select = 3
  [../]
  [./stress_xx_qp_4]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_4
    index = 0
    qp_select = 4
  [../]
  [./stress_xx_qp_5]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_5
    index = 0
    qp_select = 5
  [../]
  [./stress_xx_qp_6]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_6
    index = 0
    qp_select = 6
  [../]
  [./stress_xx_qp_7]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_7
    index = 0
    qp_select = 7
  [../]
  [./stress_xx_qp_8]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx_qp_8
    index = 0
    qp_select = 8
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_y
    boundary = 5
    value = 0.0
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y

    youngs_modulus = 1e6
    bulk_modulus = 0.333333333333333e6

    formulation = NonlinearPlaneStrain
  [../]

  [./density]
    type = Density
    block = 1
    density = 2
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  file_base = gravity_qp_select_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
