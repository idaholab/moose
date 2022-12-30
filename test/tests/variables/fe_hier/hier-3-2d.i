[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD9
[]

[Functions]
  [./bc_fnt]
    type = ParsedFunction
    expression = 3*y*y
  [../]
  [./bc_fnb]
    type = ParsedFunction
    expression = -3*y*y
  [../]
  [./bc_fnl]
    type = ParsedFunction
    expression = -3*x*x
  [../]
  [./bc_fnr]
    type = ParsedFunction
    expression = 3*x*x
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -6*x-6*y+(x*x*x)+(y*y*y)
  [../]

  [./solution]
    type = ParsedGradFunction
    expression = (x*x*x)+(y*y*y)
    grad_x = 3*x*x
    grad_y = 3*y*y
  [../]
[]

[Variables]
  [./u]
    order = THIRD
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
  [./bc_top]
    type = FunctionNeumannBC
    variable = u
    boundary = 'top'
    function = bc_fnt
  [../]
  [./bc_bottom]
    type = FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bc_fnb
  [../]
  [./bc_left]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = bc_fnl
  [../]
  [./bc_right]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = bc_fnr
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
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
