l = 2

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 20
    ymin = '${fparse -l / 2}'
    ymax = '${fparse l / 2}'
    nx = 20
    ny = 5
  []
  uniform_refine = 0
[]

[Variables]
  [diffused]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = diffused
  []

  [diff]
    type = Diffusion
    variable = diffused
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = diffused
    boundary = 'left'
    value = 1
  []

  [right]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 0
  []


  [bottom]
    type = DirichletBC
    variable = diffused
    value = 1
    boundary = 'bottom'
  []
[]



[Executioner]
  type = Transient
  num_steps = 110
  dt = 0.25
[]

[Controls]
  [web_server]
    type = WebServerControl
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END'
  []
[]


[Postprocessors]
  [diffused]
    type = ElementAverageValue
    variable = diffused
  []
[]

[Reporters]
  [constant]
    type = ConstantReporter
    real_names = 'pi'
    real_values = '${fparse pi}'
  []
[]

[Outputs]
  exodus = false
[]
