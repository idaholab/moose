[GlobalParams]
  block = '1'
[]

[Problem]
  kernel_coverage_check = 'only_list'
  kernel_coverage_block_list = '1'
  material_coverage_check = 'only_list'
  material_coverage_block_list = '1'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.25 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    bottom_left = '0.25 0 0'
    top_right = '1 1 1'
  []
[]

[MeshModifiers]
  [moving_circle]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    criterion_type = 'BELOW'
    threshold = 0
    subdomain_id = 1
    moving_boundaries = 'moving_boundary'
    moving_boundary_subdomain_pairs = '1 2; 1'
    block = '1 2'
    execute_on = 'INITIAL TIMESTEP_END'

    # --- new for setting IC --- #
    old_subdomain_reinitialized = false
    reinitialize_subdomains = '1'
    reinitialization_strategy = "IC"
    reinitialize_variables = "diff diff2 diff3"
    restore_overridden_dofs = "true"
  []
[]

[AuxVariables]
  [phi]
    block = '1 2'
  []
  [proc]
    block = '1 2'
  []
[]

[Functions]
  [moving_circle_func]
    type = ParsedFunction
    expression = (x-t)^2+(y)^2-0.5^2
  []
[]

[AuxKernels]
  [phi_kernel]
    type = FunctionAux
    variable = phi
    function = moving_circle_func
    block = '1 2'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
    block = '1 2'
  []
[]

[Variables]
  [diff]
    order = FIRST
    initial_condition = 10
  []
  [diff2]
    order = FIRST
    initial_condition = 20
  []
  [diff3]
    order = FIRST
    initial_condition = 50
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = 'diff'
    diffusivity = 'k'
  []
  [diffusion2]
    type = MatDiffusion
    variable = 'diff2'
    diffusivity = 'k'
  []
  [diffusion3]
    type = MatDiffusion
    variable = 'diff3'
    diffusivity = 'k'
  []
[]

[Materials]
  [material]
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = 26.0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = 'diff'
    boundary = left
    value = 10
  []

  [bottom]
    type = DirichletBC
    variable = 'diff'
    boundary = bottom
    value = 0
  []

  [left2]
    type = DirichletBC
    variable = 'diff2'
    boundary = left
    value = 10
  []

  [bottom2]
    type = DirichletBC
    variable = 'diff2'
    boundary = bottom
    value = 0
  []

  [bottom3]
    type = DirichletBC
    variable = 'diff3'
    boundary = bottom
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.3
  num_steps = 3
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
[]

[Outputs]
  exodus = true
[]
