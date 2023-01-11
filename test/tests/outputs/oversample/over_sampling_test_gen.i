[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 3
  ny = 3
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -4+(x*x+y*y)
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  active = 'ie diff ffn'

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

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 0.2
  start_time = 0
  num_steps = 5
[]

[Outputs]
  file_base = out_gen
  exodus = true
  [./oversampling]
    file_base = out_gen_oversample
    type = Exodus
    refinements = 3
  [../]
[]
