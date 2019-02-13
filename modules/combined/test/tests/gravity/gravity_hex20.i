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
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = SECOND
  family = LAGRANGE
[]

[Mesh]
  file = gravity_hex20_test.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
  generate_output = 'stress_xx'
[]

[Kernels]
  [./gravity]
    type = Gravity
    variable = disp_x
    value = 20
  [../]
[]

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
[]

[Materials]
  [./elasticty_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    bulk_modulus = 0.333333333333333e6
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]

  [./density]
    type = Density
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

  [./Quadrature]
    order = THIRD
  [../]
[]

[Outputs]
  file_base = gravity_hex20_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
