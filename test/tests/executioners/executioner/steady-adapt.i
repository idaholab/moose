[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = -4
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))
  [../]
[]

[Kernels]
  active = 'diff ffn'

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
  active = 'all'

  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  [./Adaptivity]
    steps = 3
    coarsen_fraction = 0.1
    refine_fraction = 0.2
    max_h_level = 5
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_steady_adapt
  exodus = true
  print_mesh_changed_info = true
[]
