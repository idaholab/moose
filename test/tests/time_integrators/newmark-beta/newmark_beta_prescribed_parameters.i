###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of the TimeIntegrator system.
#
# Testing that the first and second time derivatives
# are calculated correctly using the Newmark-Beta method
#
# @Requirement F1.30
###########################################################

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 1
  ny = 1
[]

[Variables]
  [u]
  []
[]

[Functions]
  [forcing_fn]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2    0.3  0.4    0.5  0.6'
    y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
  []
[]

[Kernels]
  [ie]
    type = TimeDerivative
    variable = u
  []

  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left'
    function = forcing_fn
  []
  [right]
    type = FunctionDirichletBC
    variable = u
    boundary = 'right'
    function = forcing_fn
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 6
  dt = 0.1
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.4225
    gamma = 0.8
  []
[]

[Postprocessors]
  [udot]
    type = ElementAverageTimeDerivative
    variable = u
  []
  [udotdot]
    type = ElementAverageSecondTimeDerivative
    variable = u
  []
  [u]
    type = ElementAverageValue
    variable = u
  []
[]

[Outputs]
  csv = true
[]
