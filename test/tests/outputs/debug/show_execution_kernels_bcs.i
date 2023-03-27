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
  [forcing_fnu]
    type = ParsedFunction
    value = -5.8*(x+y)+x*x*x-x+y*y*y-y
  []
  [forcing_fnv]
    type = ParsedFunction
    value = -4
  []

  [slnu]
    type = ParsedGradFunction
    value = x*x*x-x+y*y*y-y
    grad_x = 3*x*x-1
    grad_y = 3*y*y-1
  []
  [slnv]
    type = ParsedGradFunction
    value = x*x+y*y
    grad_x = 2*x
    grad_y = 2*y
  []

  #NeumannBC functions
  [bc_fnut]
    type = ParsedFunction
    value = 3*y*y-1
  []
  [bc_fnub]
    type = ParsedFunction
    value = -3*y*y+1
  []
  [bc_fnul]
    type = ParsedFunction
    value = -3*x*x+1
  []
  [bc_fnur]
    type = ParsedFunction
    value = 3*x*x-1
  []
[]

[Variables]
  [u]
    order = THIRD
    family = HIERARCHIC
  []
  [v]
    order = SECOND
    family = LAGRANGE
  []
[]

[Kernels]
  [diff1]
    type = Diffusion
    variable = u
  []
  [test1]
    type = CoupledConvection
    variable = u
    velocity_vector = v
  []
  [diff2]
    type = Diffusion
    variable = v
  []
  [react]
    type = Reaction
    variable = u
  []

  [forceu]
    type = BodyForce
    variable = u
    function = forcing_fnu
  []
  [forcev]
    type = BodyForce
    variable = v
    function = forcing_fnv
  []
[]

[BCs]
  [bc_v]
    type = FunctionDirichletBC
    variable = v
    function = slnv
    boundary = 'left right top bottom'
  []

  [bc_u_tb]
    type = CoupledKernelGradBC
    variable = u
    var2 = v
    vel = '0.1 0.1'
    boundary = 'top bottom left right'
  []

  [bc_ul]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnul
    boundary = 'left'
  []
  [bc_ur]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnur
    boundary = 'right'
  []
  [bc_ut]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnut
    boundary = 'top'
  []
  [bc_ub]
    type = FunctionNeumannBC
    variable = u
    function = bc_fnub
    boundary = 'bottom'
  []
[]

[Dampers]
  active = ''
  [limit_v]
    type = BoundingValueElementDamper
    variable = v
    max_value = 1.5
    min_value = -20
  []
  [limit_u]
    type = BoundingValueElementDamper
    variable = u
    max_value = 1.5
    min_value = -20
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-13
[]

[Debug]
  show_execution_order = 'NONE ALWAYS INITIAL NONLINEAR LINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
[]
