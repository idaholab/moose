###########################################################
# This is a simple test of the Kernel System.
# It solves the Laplacian equation on a small 2x2 grid.
# The "Diffusion" kernel is used to calculate the
# residuals of the weak form of this operator.
#
# @Requirement F3.30
###########################################################

AD = ''

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ${AD}Diffusion
    variable = u
  []
[]

[BCs]
  # BCs cannot be preset due to Jacobian test
  [left]
    type = ${AD}DirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  []
  [right]
    type = ${AD}DirichletBC
    variable = u
    preset = false
    boundary = right
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
