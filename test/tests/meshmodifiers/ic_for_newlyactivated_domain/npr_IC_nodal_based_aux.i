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

[UserObjects]
  [extrapolation_patch]
    type = NodalPatchRecoveryVariable
    patch_polynomial_order = FIRST
    use_specific_elements = true
    var = 'diff'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [u_patch]
    type = NodalPatchRecoveryVariable
    patch_polynomial_order = FIRST
    use_specific_elements = true
    var = 'u_recovered'
    execute_on = 'INITIAL TIMESTEP_END'
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

    reinitialization_strategy = "POLYNOMIAL_NEIGHBOR"
    reinitialize_variables = 'u_recovered'
    polynomial_fitters = 'u_patch'
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
  [u_recovered]
    family = LAGRANGE
    order = FIRST
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
  [u_recovered]
    type = ParsedAux
    variable = u_recovered
    coupled_variables = 'diff'
    expression = 'diff^2 -diff-0.5'
    execute_on = 'TIMESTEP_END'
  []
[]

[Variables]
  [diff]
    order = FIRST
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = diff
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
    variable = diff
    boundary = left
    value = 10
  []

  [bottom]
    type = DirichletBC
    variable = diff
    boundary = bottom
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.3
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
