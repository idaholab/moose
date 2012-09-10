[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 20
  ny = 20
  elem_type = QUAD9
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = (x*x+y*y)-4*(t-1)
  [../]
  [./bc_fn]
    type = ParsedFunction
    value = (x*x+y*y)*(t-1)
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./forcing_fn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = bc_fn
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.2
  num_steps = 10
  # get 'er done
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-14
[]

[Output]
  output_initial = true
  exodus = true
[]
