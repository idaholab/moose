###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of the TimeIntegrator system.
#
# Testing that the second time derivative is calculated
# correctly using the Central Difference method for an AD
# variable.
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
  [./u]
  [../]
[]

[Functions]
  [./forcing_fn]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2    0.3  0.4    0.5  0.6'
    y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
  [../]
[]

[Kernels]
  [./ie]
    type = ADTimeDerivative
    variable = u
  [../]

  [./diff]
    type = ADDiffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = ADFunctionDirichletBC
    variable = u
    boundary = 'left'
    function = forcing_fn
    preset = false
  [../]
  [./right]
    type = ADFunctionDirichletBC
    variable = u
    boundary = 'right'
    function = forcing_fn
    preset = false
  [../]
[]

[Executioner]
  type = Transient

  [./TimeIntegrator]
    type = CentralDifference
  []

  start_time = 0.0
  num_steps = 6
  dt = 0.1
[]

[Postprocessors]
  [./udotdot]
    type = ElementAverageSecondTimeDerivative
    variable = u
  [../]
[]

[Outputs]
  csv = true
[]
