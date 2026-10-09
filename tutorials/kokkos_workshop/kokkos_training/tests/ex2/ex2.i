# Exercise 2: every fix from the spot-the-bug snippets in one kernel, solving
# -div(grad(u)) + exp(u) - p(v) = 0 with p(v) = 1 + 2 v and v = x
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [reaction]
    type = KokkosPolynomialReaction
    variable = u
    v = v
    coefficients = '1 2'
  []
[]

[BCs]
  [zero]
    type = KokkosDirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Postprocessors]
  [u_integral]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
  []
  [u_max]
    type = KokkosElementExtremeValue
    variable = u
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
