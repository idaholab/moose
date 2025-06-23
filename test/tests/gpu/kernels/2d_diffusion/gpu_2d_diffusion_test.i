###########################################################
# This is a simple test of the GPUKernel System.
# It solves the Laplacian equation on a small 2x2 grid.
# The "GPUDiffusion" kernel is used to calculate the
# residuals of the weak form of this operator.
#
# @Requirement F3.30
###########################################################

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[GPUKernels]
  active = 'diff'

  [./diff]
    type = GPUDiffusion
    variable = u
  [../]
[]

[GPUBCs]
  # BCs cannot be preset due to Jacobian test
  active = 'left right'

  [./left]
    type = GPUDirichletBC
    variable = u
    preset = false
    boundary = 3
    value = 0
  [../]

  [./right]
    type = GPUDirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = out_gpu
  exodus = true
[]
