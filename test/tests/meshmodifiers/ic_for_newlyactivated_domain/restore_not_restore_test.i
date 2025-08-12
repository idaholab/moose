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
    variable = 'diff'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [extrapolation_patch_not_restore]
    type = NodalPatchRecoveryVariable
    patch_polynomial_order = FIRST
    use_specific_elements = true
    variable = 'diff_not_restore'
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
    reinitialization_strategy = "POLYNOMIAL_NEIGHBOR POLYNOMIAL_NEIGHBOR IC IC"
    reinitialize_variables = 'diff diff_not_restore diff_ic_const diff_ic_const_not_restore'
    old_subdomain_reinitialized = false
    reinitialize_subdomains = '1'
    polynomial_fitters = 'extrapolation_patch extrapolation_patch_not_restore'
    restore_overridden_dofs = "true false true false"
  []
[]

[AuxVariables]
  [phi]
    block = '1 2'
  []
  [proc]
    block = '1 2'
  []
  [proc_elem]
    block = '1 2'
    order = CONSTANT
    family = MONOMIAL
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
  [proc_elem]
    type = ProcessorIDAux
    variable = proc_elem
    execute_on = initial
    block = '1 2'
  []
[]

[Variables]
  [diff]
    order = FIRST
  []
  [diff_not_restore]
    order = FIRST
  []
  [diff_ic_const]
    order = FIRST
    initial_condition = 10
  []
  [diff_ic_const_not_restore]
    order = FIRST
    initial_condition = 10
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = diff
    diffusivity = 'k'
  []
  [diffusion_not_restore]
    type = MatDiffusion
    variable = diff_not_restore
    diffusivity = 'k'
  []
  [diff_ic_const]
    type = MatDiffusion
    variable = diff_ic_const
    diffusivity = 'k'
  []
  [diff_ic_const_not_restore]
    type = MatDiffusion
    variable = diff_ic_const_not_restore
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

  [left_not_restore]
    type = DirichletBC
    variable = diff_not_restore
    boundary = left
    value = 10
  []

  [bottom_not_restore]
    type = DirichletBC
    variable = diff_not_restore
    boundary = bottom
    value = 0
  []

  [left_ic_con]
    type = DirichletBC
    variable = diff_ic_const
    boundary = left
    value = 10
  []

  [bottom_ic_con]
    type = DirichletBC
    variable = diff_ic_const
    boundary = bottom
    value = 0
  []

  [left_ic_con_not_restore]
    type = DirichletBC
    variable = diff_ic_const_not_restore
    boundary = left
    value = 10
  []

  [bottom_ic_con_not_restore]
    type = DirichletBC
    variable = diff_ic_const_not_restore
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
