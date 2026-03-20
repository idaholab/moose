# Test problem modified from ../../problems/eigen_problem/eigensolvers/ne.i:
#
# The minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    elem_type = QUAD4
    nx = 8
    ny = 8
  []
[]



[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rhs]
    type = Reaction
    variable = u
    rate = -1.0
    extra_vector_tags = 'eigen'
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Postprocessors]
  [num_nonlin]
    type = NumNonlinearIterations
  []
  [num_lin]
    type = NumLinearIterations
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
