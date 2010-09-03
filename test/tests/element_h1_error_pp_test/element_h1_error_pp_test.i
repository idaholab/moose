[Mesh]
  dim = 2

  [./Generation]
    nx = 10
    ny = 10
    
    xmin = 0
    xmax = 2

    ymin = 0
    ymax = 2
  [../]
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
    #value = alpha*alpha*pi*pi*(y*y*sin(alpha*pi*x*y)+y*y*sin(alpha*pi*x*y))
    value = alpha*alpha*pi*pi*sin(alpha*pi*x)
    vars = 'alpha'
    vals = '4'
  [../]

  [./u_func]
    type = ParsedGradFunction
    #value = sin(alpha*pi*x*y)
    #grad_x   = alpha*pi*y*sin(alpha*pi*x*y)
    #grad_y   = alpha*pi*x*sin(alpha*pi*x*y)

    value = sin(alpha*pi*x)
    grad_x  = alpha*pi*sin(alpha*pi*x)
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
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
[]

[Postprocessors]
  [./h1]
    type = ElementH1Error
    variable = u
    function = u_func
  [../]

  [./h1_semi]
    type = ElementH1SemiError
    variable = u
    function = u_func
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = false
  postprocessor_csv = true
[]
