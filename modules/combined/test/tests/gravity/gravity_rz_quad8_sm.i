# This test uses small strain formulation, and the use_displaced_mesh
# is set to false for the kernels
#
# Gravity Test
#
# This test is designed to exercise the gravity body force rz kernel.
#
# The mesh for this problem is a rectangle 10 units by 1 unit.
#
# The boundary conditions for this problem are as follows.  The
#   displacement is zero at the top.  The acceleration of gravity is 20.
#
# The material has a Young's modulus of 1e6 and a density of 2.
#
# The analytic solution for the displacement along the bar is:
#
# u(y) = -b*y^2/(2*E)+b*L*y/E
#
# The displacement at y=L is b*L^2/(2*E) = 2*20*10*10/(2*1e6) = 0.002.
#
# The analytic solution for the stress along the bar assuming linear
#   elasticity is:
#
# S(y) = b*(L-y)
#
# The stress at x=0 is b*L = 2*20*10 = 400.
#
# Note:  The simulation does not measure stress at y=0.  The stress
#   is reported at element centers.  The element closest to y=0 sits
#   at y = 1/4 and has a stress of 390.  This matches the linear
#   stress distribution that is expected.  The same situation applies
#   at y = L where the stress is zero analytically.  The nearest
#   element is at y=9.75 where the stress is 10.
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = gravity_rz_quad8_test.e
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
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]

[Kernels]
  [./gravity]
    type = Gravity
    variable = disp_y
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
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
[]

[BCs]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

    shear_modulus = 0.5e6
    lambda = 0.0

    formulation = NonlinearRZ
  [../]

  [./density]
    type = Density
    block = 1
    density = 2
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  end_time = 1.0

  [./Quadrature]
    order = THIRD
  [../]
[]

[Outputs]
  file_base = gravity_rz_quad8_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
