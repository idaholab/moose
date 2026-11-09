# The averaged functor is u = t * x, evaluated at the element centroids x = 0.5 and 1.5
# at t = 1, 2, ..., 6 with dt = 1. Averaging is over [start_time, t] with start_time = 2.5,
# which falls inside a time step, so the sample at t = 3 only has weight 0.5. Before that the
# variable holds u. The rectangle rule average is x times
#   1, 2, 3, (0.5 * 3 + 4) / 1.5, (0.5 * 3 + 4 + 5) / 2.5, (0.5 * 3 + 4 + 5 + 6) / 3.5
#   = 1, 2, 3, 11/3, 4.2, 33/7
# The trapezoidal rule is exact for u linear in t, so the trapezoidal average is x times the
# exact mean of t over [2.5, t], (2.5 + t) / 2:
#   1, 2, 2.75, 3.25, 3.75, 4.25
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmax = 2
[]

[Problem]
  solve = false
[]

[Functions]
  [u]
    type = ParsedFunction
    expression = 't * x'
  []
[]

[AuxVariables]
  [u_avg]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [u_avg]
    type = CumulativeTimeAverageFunctorAux
    variable = u_avg
    functor = u
    start_time = 2.5
  []
[]

[Postprocessors]
  [u_avg_left]
    type = PointValue
    variable = u_avg
    point = '0.5 0 0'
  []
  [u_avg_right]
    type = PointValue
    variable = u_avg
    point = '1.5 0 0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 6
  dt = 1
[]

[Outputs]
  csv = true
[]
