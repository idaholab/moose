# Exercise 5: the Exercise 3 problem with the conductivity provided by a material. The exact
# solution is u(x) = (sqrt(1 + 8 x) - 1) / 2.
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
    type = KokkosMatNonlinearDiffusion
    variable = u
  []
[]

[Materials]
  [conductivity]
    type = KokkosLinearConductivity
    temperature = u
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
