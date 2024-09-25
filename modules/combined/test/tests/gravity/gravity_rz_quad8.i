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
  order = SECOND
  family = LAGRANGE
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = gravity_rz_quad8_test.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = FINITE
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_xy'
[]

[Kernels]
  [./gravity]
    type = Gravity
    variable = disp_y
    value = 20
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
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    shear_modulus = 0.5e6
    lambda = 0.0
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]

  [./density]
    type = Density
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
