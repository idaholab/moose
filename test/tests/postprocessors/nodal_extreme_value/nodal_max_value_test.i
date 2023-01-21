[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 20
  ny = 20
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = (sin(pi*t))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = sin(pi*t)
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff' #ffn'

  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 0.1
  start_time = 0
  num_steps = 20
[]

[Postprocessors]
  [./max_nodal_val]
    type = NodalMaxValue
    variable = u
  [../]
[]

[Outputs]
  file_base = out_nodal_max
  exodus = true
[]
