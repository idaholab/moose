# Gravity Test
#
# This test is designed to exercise the gravity body force kernel.
#
# The mesh for this problem is a rectangular bar 10 units by 1 unit
#   by 1 unit.
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
# The displacement at x=L is b*L^2/(2*E) = 2*20*10*10/(2*1e6) = 0.002.
#
# The analytic solution for the stress along the bar assuming linear
#   elasticity is:
#
# S(x) = b*(L-x)
#
# The stress at x=0 is b*L = 2*20*10 = 400.
#
# Note:  The simulation does not measure stress at x=0.  The stress
#   is reported at element centers.  The element closest to x=0 sits
#   at x = 1/4 and has a stress of 390.  This matches the linear
#   stress distribution that is expected.  The same situation applies
#   at x = L where the stress is zero analytically.  The nearest
#   element is at x=9.75 where the stress is 10.
#

[Mesh]#Comment
  file = gravity_hex20_test.e
  displacements = 'disp_x disp_y disp_z'
[] # Mesh

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

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Kernels]

  [./gravity]
    type = Gravity
    variable = disp_x
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

[] # AuxKernels

[BCs]

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 5
    value = 0.0
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    bulk_modulus = 0.333333333333333e6
  [../]

  [./density]
    type = Density
    block = 1
    density = 2
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'


  line_search = 'none'


  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  l_max_its = 30

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0

  [./Quadrature]
    order = THIRD
  [../]

[] # Executioner

[Outputs]
  file_base = out_hex20
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
