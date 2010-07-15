[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 5
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'forcing_func u_func'
  
  [./forcing_func]
    type = ParsedFunction
    #function = alpha*alpha*pi*pi*(y*y*sin(alpha*pi*x*y)+y*y*sin(alpha*pi*x*y))
    function = alpha*alpha*pi*pi*sin(alpha*pi*x)
    vars = 'alpha'
    vals = '4'
  [../]

  [./u_func]
    type = ParsedGradFunction
    #function = sin(alpha*pi*x*y)
    #grad_x   = alpha*pi*y*sin(alpha*pi*x*y)
    #grad_y   = alpha*pi*x*sin(alpha*pi*x*y)

    function = sin(alpha*pi*x)
    grad_x   = alpha*pi*sin(alpha*pi*x)
    vars = 'alpha'
    vals = '4'
  [../]
[]

[Kernels]
  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = '2'
    value = 0
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
[]

[Postprocessors]
  [./integral]
    type = ElementH1Error
    variable = u
    function = u_func
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
