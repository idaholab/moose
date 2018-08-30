# This test demonstrates that a Problem can be created through an Action (possibly associated with
# special syntax), that may or may not even have a type specified.
# See the custom "TestProblem" block below.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[TestProblem]
  # Creates a custom problem through a meta-action.
  name = 'MOOSE Action Test problem'
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
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
[]
