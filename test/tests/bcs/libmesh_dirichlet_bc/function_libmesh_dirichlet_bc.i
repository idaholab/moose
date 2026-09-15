# Laplace's equation with u = t + x prescribed on every boundary and no forcing. That function is
# harmonic and linear, so it is the exact solution and it lies in the vertex subspace of a
# hierarchic basis. Over the unit square the average is then exactly t + 0.5, and the L2 error
# against the exact solution is zero to round-off, for any family and order.
#
# The time dependence is the point: the prescribed values are reprojected once per solve, at MOOSE's
# current time, so a wrong time here would show up directly in both postprocessors.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    family = HIERARCHIC
    order = SECOND
  []
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = 't + x'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [all]
    type = FunctionLibmeshDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = ramp
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.5
  solve_type = NEWTON
  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
  [l2_err]
    type = ElementL2Error
    variable = u
    function = ramp
  []
[]

[Outputs]
  csv = true
[]
