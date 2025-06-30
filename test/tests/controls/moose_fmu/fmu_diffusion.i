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
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.5
[]

[Controls]
  [web_server]
    type = WebServerControl
    execute_on = 'TIMESTEP_BEGIN'
    port = 40000
  []
[]


[Postprocessors]
  [t]
    type = FunctionValuePostprocessor
    function = 't'
  []

  [dt]
    type = TimestepSize
  []

  [diffused]
    type = ElementAverageValue
    variable = diffused
  []
[]

[Outputs]
  exodus = true
[]
