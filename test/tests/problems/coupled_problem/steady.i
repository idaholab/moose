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
    value = 4
  [../]
  [./bc_fn]
    type = ParsedFunction
    value = -(x*x+y*y)
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
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
  type = Steady
  # get 'er done
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-14

  solve_type = NEWTON
[]

[Output]
  output_initial = false
  exodus = true
[]
