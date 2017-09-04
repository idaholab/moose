[GlobalParams]
  gravity = '0 0 0'
  stabilize = true
  convective_term = false
  integrate_p_by_parts = true
  laplace = true
  u = vel_x
  v = vel_y
  p = p
  alpha = 1e-6
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1.0
  ymin = 0
  ymax = 1.0
  nx = 32
  ny = 32
  elem_type = QUAD9
[]

[MeshModifiers]
  [./corner_node]
    type = AddExtraNodeset
    new_boundary = 'pinned_node'
    nodes = '0'
  [../]
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]

  [./p]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
  [../]

  [./mass_pspg]
    type = INSMassPSPG
    variable = p
    u = vel_x
    v = vel_y
    p = p
  [../]

  # # x-momentum, time
  # [./x_momentum_time]
  #   type = INSMomentumTimeDerivative
  #   variable = vel_x
  # [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentumChild
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
  [../]

  # # y-momentum, time
  # [./y_momentum_time]
  #   type = INSMomentumTimeDerivative
  #   variable = vel_y
  # [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentumChild
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
  [../]

  [./vel_x_source]
    type = UserForcingFunction
    function = vel_x_source_func
    variable = vel_x
  [../]

  [./vel_y_source]
    type = UserForcingFunction
    function = vel_y_source_func
    variable = vel_y
  [../]

  [./p_source]
    type = UserForcingFunction
    function = p_source_func
    variable = p
  [../]
[]

[BCs]
  [./vel_x]
    type = FunctionDirichletBC
    boundary = 'left right top bottom'
    function = vel_x_func
    variable = vel_x
  [../]
  [./vel_y]
    type = FunctionDirichletBC
    boundary = 'left right top bottom'
    function = vel_y_func
    variable = vel_y
  [../]
  [./p]
    type = FunctionDirichletBC
    boundary = 'left right top bottom'
    function = p_func
    variable = p
  [../]
[]

[Functions]
  [./vel_x_source_func]
    type = ParsedFunction
    value = ''
  [../]
  [./vel_y_source_func]
    type = ParsedFunction
    value = ''
  [../]
  [./p_source_func]
    type = ParsedFunction
    value = ''
  [../]
  [./vel_x_func]
    type = ParsedFunction
    value = ''
  [../]
  [./vel_y_func]
    type = ParsedFunction
    value = ''
  [../]
  [./p_func]
    type = ParsedFunction
    value = ''
  [../]
  [./vxx_func]
    type = ParsedFunction
    value = ''
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  [../]
[]

[Functions]
  [./lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    value = '4*x*(1-x)'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = FDP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  # Run for 100+ timesteps to reach steady state.
  # num_steps = 5
  # dt = .5
  # dtmin = .5
  # petsc_options = '-snes_test_display'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  # petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels'
  # petsc_options_value = 'asm      2               ilu          4'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  [./exodus]
    type = Exodus
    file_base = 'forward_test'
  [../]
  [./csv]
    type = CSV
    file_base = 'forward_test'
  [../]
[]

# [ICs]
#   [./vel_x]
#     type = RandomIC
#     variable = vel_x
#     min = .1
#     max = .9
#   [../]
#   [./vel_y]
#     type = RandomIC
#     variable = vel_y
#     min = .1
#     max = .9
#   [../]
#   [./p]
#     type = RandomIC
#     variable = p
#     min = .1
#     max = .9
#   [../]
# []

[Postprocessors]
  [./L2vel_x]
    type = ElementL2Error
    variable = vel_x
    function = vel_x_func
    outputs = 'console csv'
  [../]
  [./L2vel_y]
    variable = vel_y
    function = vel_y_func
    type = ElementL2Error
    outputs = 'console csv'
  [../]
  [./L2p]
    variable = p
    function = p_func
    type = ElementL2Error
    outputs = 'console csv'
  [../]
  [./L2vxx]
    variable = vxx
    function = vxx_func
    type = ElementL2Error
    outputs = 'console csv'
  [../]
  # [./L2nvel_x]
  #   type = NodalL2Error
  #   variable = vel_x
  #   function = vel_x_func
  #   outputs = 'console csv'
  # [../]
  # [./L2nvel_y]
  #   variable = vel_y
  #   function = vel_y_func
  #   type = NodalL2Error
  #   outputs = 'console csv'
  # [../]
  # [./L2np]
  #   variable = p
  #   function = p_func
  #   type = NodalL2Error
  #   outputs = 'console csv'
  # [../]
[]

[AuxVariables]
  [./vxx]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[AuxKernels]
  [./vxx]
    type = VariableGradientComponent
    component = x
    variable = vxx
    gradient_variable = vel_x
  [../]
[]
