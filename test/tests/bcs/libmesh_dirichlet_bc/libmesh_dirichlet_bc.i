# Laplace's equation with a constant value on every boundary and no forcing. The exact solution is
# that same constant, which lies in the vertex subspace of a hierarchic basis, so every bubble
# coefficient of the exact answer is zero and both postprocessors below are exactly the boundary
# value for any family and order.

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

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [all]
    type = LibmeshDirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
  [max_u]
    type = ElementExtremeValue
    variable = u
  []
[]

[Outputs]
  csv = true
[]
