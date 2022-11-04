[Mesh]
  type = GeneratedMesh
  dim  = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx   = 4
  ny   = 4
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*t*t*((x*x)+(y*y))
  [../]
[]

[Kernels]
  active = 'diff ie ffn'

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
  active = 'all'

  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value    = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value    = 1
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
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  start_time = 0.0
  num_steps  = 5
  dt         = 0.25

  [./TimeIntegrator]
    type = LStableDirk2
  [../]

  [./Adaptivity]
    refine_fraction  = 0.07
    coarsen_fraction = 0.
    max_h_level      = 4
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
