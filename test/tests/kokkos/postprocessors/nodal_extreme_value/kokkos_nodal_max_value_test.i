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
  [exact_fn]
    type = ParsedFunction
    expression = (sin(pi*t))
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 0.1
  start_time = 0
  num_steps = 20
[]

[Postprocessors]
  [max_nodal_val]
    type = KokkosNodalExtremeValue
    variable = u
  []
[]

[Outputs]
  exodus = true
[]
