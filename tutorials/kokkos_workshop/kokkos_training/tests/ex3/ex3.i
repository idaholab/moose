# Exercise 3: nonlinear diffusion -div(k(u) grad(u)) = 0 with k(u) = k0 (1 + beta u) on the unit
# square, u = 0 on the left and u = 1 on the right. With k0 = 1 and beta = 2 the exact solution is
# u(x) = (sqrt(1 + 8 x) - 1) / 2, so u(0.5) = (sqrt(5) - 1) / 2 = 0.618034.
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
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = '(sqrt(1 + 8 * x) - 1) / 2'
  []
[]

[Postprocessors]
  [u_mid]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  []
  [l2_error]
    type = ElementL2Error
    variable = u
    function = exact
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
