[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 10
    ny = 10
    elem_type = QUAD9
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = 'gmg'
    combinatorial_geometry = 'x < 0.5'
    block_id = '2'
  []
  [middle_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'left'
    primary_block = '0'
    paired_block = '2'
    new_boundary = 'middle'
  []
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

  # NeumannBC functions
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

[InterfaceKernels]
  [diff_ik_2]
    type = InterfaceDiffusion
    variable = 'u'
    neighbor_var = 'v'
    boundary = 'middle'
  []
  [diff_ik_1]
    type = InterfaceDiffusion
    variable = 'v'
    neighbor_var = 'u'
    boundary = 'middle'
  []
[]

[DGKernels]
  [diff_dg_2]
    type = DGDiffusion
    variable = 'u'
    epsilon = -1
    sigma = 6
  []
  [diff_dg_1]
    type = DGDiffusion
    variable = 'u'
    epsilon = -1
    sigma = 6
  []
[]

[DiracKernels]
  [source_2]
    type = FunctionDiracSource
    variable = 'u'
    point = '0.1 0.1 0'
    function = 'x + y'
  []
  [source_1]
    type = FunctionDiracSource
    variable = 'u'
    point = '0.1 0.1 0'
    function = 'x + y'
    block = '2'
  []
  [source_0]
    type = FunctionDiracSource
    variable = 'u'
    # in block 0, but since it's not block restricted it shows up as active in
    # block 2 as well
    point = '0.6 0.5 0'
    function = 'x + y'
  []
[]

[Materials]
  [diff]
    type = GenericConstantMaterial
    prop_names = 'D D_neighbor'
    prop_values = '0 0'
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
