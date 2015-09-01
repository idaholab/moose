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

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = gravity_rz_test.e
  displacements = 'disp_x disp_y'
[] # Mesh

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
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

[] # AuxVariables

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

[] # Kernels

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

[] # AuxKernels

[BCs]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

    shear_modulus = 0.5e6
    lambda = 0.0
  [../]

  [./density]
    type = Density
    block = 1
    density = 2
    disp_r = disp_x
    disp_z = disp_y
  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  file_base = out_rz
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
