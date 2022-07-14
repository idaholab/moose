[Mesh]
  [geo]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
  [material_domain]
    input = geo
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    block_id = 1
    block_name = material
    top_right = '0.5 1 0'
  []
  [void_domain]
    input = material_domain
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    block_id = 2
    block_name = void
    top_right = '1 1 0'
  []
  [interface_12]
    type = SideSetsBetweenSubdomainsGenerator
    input = void_domain
    primary_block = 1
    paired_block = 2
    new_boundary = 'phase_interface_12'
  []
  [interface_21]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface_21
    primary_block = 2
    paired_block = 1
    new_boundary = 'phase_interface_21'
  []
[]

[UserObjects]
  [material_recession] # object controlling the subdomain shift
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = phi
    block = 1
    criterion_type = BELOW
    threshold = 9
    subdomain_id = 2
    moving_boundary_name = 'phase_interface_21'
    complement_moving_boundary_name = 'phase_interface_12'
    apply_initial_conditions = false
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Variables]
  [phi]
    block = 'material'
  []
[]

[Kernels]
  [mass_diffusion]
    type = ADMatDiffusion
    variable = phi
    diffusivity = 1e-5
    block = 'material'
  []
  [mass_diffusion_time]
    type = ADTimeDerivative
    variable = phi
    block = 'material'
  []
[]

[BCs]
  [flux]
    type = NeumannBC
    boundary = 'phase_interface_12'
    variable = phi
    value = -5e-3
  []
[]

[ICs]
  [init_phi]
    type = ConstantIC
    block = 'material'
    value = 10
    variable = phi
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 3
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-08
[]

[Postprocessors]
  [surface_flux]
    type = SideDiffusiveFluxAverage
    variable = phi
    boundary = 'phase_interface_12'
    diffusivity = 1e-5
  []
[]

[Outputs]
  exodus = true
[]
