# The averaged functor is u = t * x, evaluated at the element centroids x = 0.5 and 1.5
# at t = 1, 2, ..., 6 with dt = 1. Averaging starts at start_time = 2, so the average
# is x times the mean of the sampled times since t = 2:
#   x = 0.5 : 0.5, 1, 1.25, 1.5, 1.75, 2
#   x = 1.5 : 1.5, 3, 3.75, 4.5, 5.25, 6
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
    type = RunningTimeAverageAux
    variable = u_avg
    functor = u
    start_time = 2
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
