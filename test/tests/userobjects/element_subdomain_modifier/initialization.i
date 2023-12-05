[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
  []
  [middle]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    block_name = middle
    bottom_left = '0.25 0 0'
    top_right = '0.75 1 1'
  []
  allow_renumbering = false
[]

[UserObjects]
  [moving_circle]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    block = middle
    criterion_type = BELOW
    threshold = 0
    subdomain_id = 0
    active_subdomains = 0
    initialize_variables = 'u v w s'
    initialization_strategy = 'IC NEAREST CONSTANT CONSTANT'
    initialization_constant = '0.213 -0.12' # There are two variables with initialization_strategy==CONSTANT, so we need to constants here.
    moving_boundary_name = moving_boundary
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Functions]
  [moving_circle]
    type = ParsedFunction
    expression = '(x-0.5)^2+(y-t)^2-0.5^2'
  []
[]

[Variables]
  [u]
    block = 0
  []
[]

[AuxVariables]
  [v]
    block = 0
  []
  [w]
    block = 0
  []
  [s]
    block = 0
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
    block = 0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x'
    block = 0
  []
  [v]
    type = FunctionIC
    variable = v
    function = 'x'
    block = 0
  []
  [w]
    type = FunctionIC
    variable = w
    function = 'y'
    block = 0
  []
  [s]
    type = FunctionIC
    variable = s
    function = 'x+y'
    block = 0
  []
[]

[AuxVariables]
  [phi]
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = moving_circle
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.02
  end_time = 0.2
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
