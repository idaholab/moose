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
  [./forcing_fnu]
    type = ParsedFunction
    expression = -6*(x+y)+x*x+y*y
  [../]
  [./forcing_fnv]
    type = ParsedFunction
    expression = -4+x*x*x-x+y*y*y-y
  [../]
  [./bc_fnut]
    type = ParsedFunction
    expression = 3*y*y-1
  [../]
  [./bc_fnub]
    type = ParsedFunction
    expression = -3*y*y+1
  [../]
  [./bc_fnul]
    type = ParsedFunction
    expression = -3*x*x+1
  [../]
  [./bc_fnur]
    type = ParsedFunction
    expression = 3*x*x-1
  [../]
  [./slnu]
    type = ParsedGradFunction
    expression = x*x*x-x+y*y*y-y
    grad_x = 3*x*x-1
    grad_y = 3*y*y-1
  [../]
  [./slnv]
    type = ParsedGradFunction
    value = x*x+y*y
    grad_x = 2*x
    grad_y = 2*y
  [../]
[]

[Variables]
  [./u]
    order = THIRD
    family = HIERARCHIC
  [../]
  [./v]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff1]
    type = Diffusion
    variable = u
  [../]
  [./diff2]
    type = Diffusion
    variable = v
  [../]
  [./forceu]
    type = BodyForce
    variable = u
    function = forcing_fnu
  [../]
  [./forcev]
    type = BodyForce
    variable = v
    function = forcing_fnv
  [../]
[]

[BCs]
  # active = 'bc_u bc_v'
  # [./bc_u]
  # type = FunctionDirichletBC
  # variable = u
  # function = slnu
  # boundary = 'top left right bottom'
  # [../]
  [./bc_ut]
    type = FunctionDirichletBC
    variable = u
    boundary = top
    function = bc_fnut
  [../]
  [./bc_ub]
    type = FunctionDirichletBC
    variable = u
    boundary = bottom
    function = bc_fnub
  [../]
  [./bc_ul]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = bc_fnul
  [../]
  [./bc_ur]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = bc_fnur
  [../]
  [./bc_v]
    type = FunctionDirichletBC
    variable = v
    function = slnv
    boundary = 'top left right bottom'
  [../]
[]

[Preconditioning]
  [./prec]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  active = 'num_vars'
  [./dofs]
    type = NumDOFs
  [../]
  [./h]
    type = AverageElementSize
  [../]
  [./L2u]
    type = ElementL2Error
    variable = u
    function = slnu
  [../]
  [./L2v]
    type = ElementL2Error
    variable = v
    function = slnv
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
  [./num_vars]
    type = NumVars
    system = 'NL'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-15

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
