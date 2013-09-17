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
    value = -6*(x+y)+x*x+y*y
  [../]
  [./forcing_fnv]
    type = ParsedFunction
    value = -4+x*x*x-x+y*y*y-y
  [../]
  [./bc_fnut]
    type = ParsedFunction
    value = 3*y*y-1
  [../]
  [./bc_fnub]
    type = ParsedFunction
    value = -3*y*y+1
  [../]
  [./bc_fnul]
    type = ParsedFunction
    value = -3*x*x+1
  [../]
  [./bc_fnur]
    type = ParsedFunction
    value = 3*x*x-1
  [../]
  [./slnu]
    type = ParsedGradFunction
    value = x*x*x-x+y*y*y-y
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
  [./test1]
    type = CoupledKernelValueTest
    variable = u
    var2 = v
  [../]
  [./diff2]
    type = Diffusion
    variable = v
  [../]
  [./test2]
    type = CoupledKernelValueTest
    variable = v
    var2 = u
  [../]
  [./forceu]
    type = UserForcingFunction
    variable = u
    function = forcing_fnu
  [../]
  [./forcev]
    type = UserForcingFunction
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
    type = FunctionNeumannBC
    variable = u
    boundary = top
    function = bc_fnut
  [../]
  [./bc_ub]
    type = FunctionNeumannBC
    variable = u
    boundary = bottom
    function = bc_fnub
  [../]
  [./bc_ul]
    type = FunctionNeumannBC
    variable = u
    boundary = left
    function = bc_fnul
  [../]
  [./bc_ur]
    type = FunctionNeumannBC
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
    variable = u
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
  [../]
[]

[Executioner]
  # petsc_options = '-snes_mf_operator'
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-15
[]

[Output]
  postprocessor_csv = true
  perf_log = true
[]

