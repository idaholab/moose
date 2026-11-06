# Exercise 4: the Exercise 3 problem with a solution-dependent convective flux on the top boundary
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosNonlinearDiffusion
    variable = u
    k0 = 1
    beta = 2
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [top]
    type = KokkosLinearConvectiveBC
    variable = u
    boundary = top
    h0 = 2
    gamma = 3
    u_inf = 0.2
  []
[]

[Postprocessors]
  [u_mid]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  []
  [u_top]
    type = KokkosSideAverageValue
    variable = u
    boundary = top
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
[]
