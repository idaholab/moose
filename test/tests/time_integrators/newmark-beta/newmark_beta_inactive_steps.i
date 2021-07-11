###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of the TimeIntegrator system.
#
# Testing that the active_time parameter works as intended.
#
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
    x = '0.0 0.1 0.6'
    y = '0.0 1.0 1.0'
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
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
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
    inactive_tsteps = 1
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
