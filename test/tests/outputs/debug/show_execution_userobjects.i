[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.5 2.4'
    dy = '1.3 0.9'
    ix = '3 2'
    iy = '2 3'
    subdomain_id = '0 1
                    1 0'
  []
  [add_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'cmg'
    primary_block = 0
    paired_block = 1
    new_boundary = 'interface'
  []
  second_order = true
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
    order = SECOND
    family = HIERARCHIC
  []
  [v]
    order = SECOND
    family = LAGRANGE
    initial_condition = 1
  []
[]

[AuxVariables]
  [v_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  active = 'diff1 diff2 test1 forceu forcev react'
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

[AuxKernels]
  [set_v_elem]
    type = FunctionAux
    variable = v_elem
    # selected not to be the solution for no particular reason
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

[Postprocessors]
  # Global user objects
  [dofs]
    type = NumDOFs
  []
  [h]
    type = AverageElementSize
  []

  # Elemental user objects
  [L2u]
    type = ElementL2Error
    variable = u
    function = slnu
    # Testing an option
    force_preic = true
  []
  [L2v]
    type = ElementL2Error
    variable = v
    function = slnv
    # Testing an option
    force_preaux = true
  []
  [H1error]
    type = ElementH1Error
    variable = u
    function = slnu
  []
  [H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = slnu
  []
  [L2v_elem]
    type = ElementL2Error
    variable = v_elem
    function = slnv
  []
  [f_integral]
    type = FunctionElementIntegral
    function = slnv
  []
  [int_v]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = 1
    execute_on = 'TIMESTEP_END transfer'
  []
  [int_v_elem]
    type = ElementIntegralVariablePostprocessor
    variable = v_elem
    block = 1
    execute_on = 'TIMESTEP_END transfer'
  []

  # Side user objects
  [integral_v]
    type = SideIntegralVariablePostprocessor
    variable = v
    boundary = 0
  []
[]

[VectorPostprocessors]
  # General UOs
  [memory]
    type = VectorMemoryUsage
  []
  [line]
    type = LineValueSampler
    variable = v
    num_points = 10
    start_point = '0 0 0'
    end_point = '0.5 0.5 0'
    sort_by = 'x'
  []

  # Nodal UOs
  [nodal_sampler_y]
    type = NodalValueSampler
    variable = v
    sort_by = 'y'
  []
  [nodal_sampler_x]
    type = NodalValueSampler
    variable = v
    sort_by = 'x'
  []

  # Element UO
  [elem_sample]
    type = ElementValueSampler
    variable = v_elem
    sort_by = 'x'
  []
[]

[UserObjects]
  # Nodal user objects
  [find_node]
    type = NearestNodeNumberUO
    point = '0.5 0.5 0'
  []

  # Side user objects
  [side_int]
    type = LayeredSideIntegral
    variable = v
    boundary = 0
    direction = y
    num_layers = 4
  []
  [side_int_2]
    type = NearestPointLayeredSideIntegral
    variable = v
    boundary = 0
    direction = x
    num_layers = 3
    points = '1 1 0'
  []

  # Interface user objects
  [values]
    type = InterfaceQpValueUserObject
    var = v
    boundary = interface
  []

  inactive = 'prime_1 prime_2'
  # Threaded general user objects
  [prime_2]
    type = PrimeProductUserObject
  []
  [prime_1]
    type = PrimeProductUserObject
  []

  # Domain user objects
  [domain_2]
    type = InterfaceDomainUserObject
    u = u
    v = v
    block = '0'
    robin_boundaries = 'left'
    interface_boundaries = 'interface'
    interface_penalty = 1e-10
    nl_abs_tol = 1e1
  []
  [domain_1]
    type = InterfaceDomainUserObject
    u = u
    v = v
    block = '0 1'
    robin_boundaries = 'left'
    interface_boundaries = 'interface'
    interface_penalty = 1e-10
    nl_abs_tol = 1e1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  l_tol = 1e-5
[]

[Problem]
  kernel_coverage_check = false
[]

[MultiApps]
  active = ''
  [full_solve]
    type = FullSolveMultiApp
    execute_on = 'initial timestep_end final'
    input_files = show_execution_userobjects.i
    cli_args = 'Problem/solve=false'
  []
[]

[Transfers]
  active = ''
  [conservative]
    type = MultiAppNearestNodeTransfer
    from_multi_app = full_solve
    source_variable = v
    variable = v_elem
    from_postprocessors_to_be_preserved = int_v
    to_postprocessors_to_be_preserved = int_v_elem
  []
[]

[Debug]
  show_execution_order = 'ALWAYS INITIAL NONLINEAR LINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
[]
