[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.141
  ymin = 3.141
  ymax = 1
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Functions]
  [./parsed_function]
    type = ParsedGradFunction
    value = 'sin(x)-cos(y/2)'
  [../]
  [./parsed_grad_function]
    type =ParsedGradFunction
    value = 'sin(x)-cos(y/2)'
    grad_x = 'cos(x)'
    grad_y = 'sin(y/2)/2'
  [../]
  [./parsed_zerograd_function]
    type = ParsedGradFunction
    value = 'sin(x)-cos(y/2)'
    grad_x = '0'
    grad_y = '0'
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'u'
    function = parsed_function
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  output_initial = true
  file_base = parsed
  [./OverSampling]
    type = Exodus
    refinements = 3
    oversample = true
  [../]
  [./console]
    type = Console
    perf_log = false
  [../]
[]
