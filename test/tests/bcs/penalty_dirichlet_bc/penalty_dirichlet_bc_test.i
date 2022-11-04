[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = -2*(x*x+y*y-2)+(1-x*x)*(1-y*y)
  [../]

  [./solution]
    type = ParsedGradFunction
    value = (1-x*x)*(1-y*y)
    grad_x = 2*(x*y*y-x)
    grad_y = 2*(x*x*y-y)
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = HIERARCHIC
  [../]
[]

[Kernels]
  active = 'diff forcing reaction'
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./reaction]
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  active = 'bc_all'
  [./bc_all]
    type = PenaltyDirichletBC
    variable = u
    value = 0
    boundary = 'top left right bottom'
    penalty = 1e5
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]

  [./h]
    type = AverageElementSize
  [../]

  [./L2error]
    type = ElementL2Error
    variable = u
    function = solution
  [../]
  [./H1error]
    type = ElementH1Error
    variable = u
    function = solution
  [../]
  [./H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = solution
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-14
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
