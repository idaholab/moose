[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Functions]
  [forcing_fn]
    type = ParsedFunction
    expression = -4
  []
  [exact_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))
  []
[]

[Kernels]
  [ie]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
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

[Postprocessors]
  [steady_norm]
    type = SolutionChangeOverTimePostprocessor
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-14

  start_time = 0.0
  num_steps = 5
  dt = 1

  steady_state_detection = true
[]

[Outputs]
  csv = true
[]
