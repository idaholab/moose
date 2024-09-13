[Problem]
  type = FEProblem
[]

[Mesh]
  [BaseMesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = -2
    xmax = +2
    ymin = -1
    ymax = +1
  []
  [Box1]
    type = SubdomainBoundingBoxGenerator
    input = "BaseMesh"
    block_id = 2
    location = "INSIDE"
    bottom_left = "-2 -1 0"
    top_right = "0 0 0"
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [q]
    family = MONOMIAL
    order = third
    block = 2
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
[]

# Request AuxVariable "q" again. This time without block restriction.
# This would widen the scope of "q" by means of block-restriction. This is not allowed.
# Note: If this block were above [AuxVariables/q], no error would occour.
[MoreAuxVariables]
  [q]
    family = MONOMIAL
    order = third
  []
[]
