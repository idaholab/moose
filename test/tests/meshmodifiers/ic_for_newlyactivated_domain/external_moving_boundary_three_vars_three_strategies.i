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
    variable = 'diff'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [extrapolation_patch2]
    type = NodalPatchRecoveryVariable
    patch_polynomial_order = FIRST
    variable = 'diff2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [extrapolation_patch3]
    type = NodalPatchRecoveryVariable
    patch_polynomial_order = FIRST
    variable = 'diff3'
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

    reinitialization_strategy = "POLYNOMIAL_NEIGHBOR POLYNOMIAL_WHOLE POLYNOMIAL_NEARBY"
    reinitialize_variables = 'diff diff2 diff3'
    nearby_distance_threshold = 0.375
    polynomial_fitters = 'extrapolation_patch extrapolation_patch2 extrapolation_patch3'
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
  []
  [diff2]
    order = FIRST
  []
  [diff3]
    order = FIRST
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

  [left3]
    type = DirichletBC
    variable = 'diff3'
    boundary = left
    value = 10
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
[]

[Outputs]
  exodus = true
[]
