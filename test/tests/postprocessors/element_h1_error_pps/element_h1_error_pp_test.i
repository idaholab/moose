[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3

  xmin = 0
  xmax = 2

  ymin = 0
  ymax = 2
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
    #grad_x   = alpha*pi*y*cos(alpha*pi*x*y)
    #grad_y   = alpha*pi*x*cos(alpha*pi*x*y)

    value = sin(alpha*pi*x)
    grad_x = alpha*pi*cos(alpha*pi*x)
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
    boundary = '3'
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  [./Adaptivity]
    refine_fraction = 1.0
    coarsen_fraction = 0.0
    max_h_level = 10
    steps = 4
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]

  [./h1_error]
    type = ElementH1Error
    variable = u
    function = u_func
  [../]

  [./h1_semi]
    type = ElementH1SemiError
    variable = u
    function = u_func
  [../]

  [./l2_error]
    type = ElementL2Error
    variable = u
    function = u_func
  [../]
[]

[Outputs]
  file_base = out
  exodus = false
  csv = true
  output_initial = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
