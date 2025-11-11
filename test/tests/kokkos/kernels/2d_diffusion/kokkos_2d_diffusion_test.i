###########################################################
# This is a simple test of the KokkosKernel System.
# It solves the Laplacian equation on a small 2x2 grid.
# The "KokkosDiffusion" kernel is used to calculate the
# residuals of the weak form of this operator.
#
# @Requirement F3.30
###########################################################

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  # BCs cannot be preset due to Jacobian test
  active = 'left right'

  [left]
    type = KokkosDirichletBC
    variable = u
    preset = false
    boundary = 3
    value = 0
  []

  [right]
    type = KokkosDirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
