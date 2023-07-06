[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    nx = 2
    ny = 1
    subdomain_ids = '0 1'
  []

  [interface_mesh]
    type = SideSetsBetweenSubdomainsGenerator
    input = gmg
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []

  # This creates enough elements to have defined gradients
  [refine]
    type = RefineBlockGenerator
    input = interface_mesh
    block = '0 1'
    refinement = '3 3'
  []
[]

[Adaptivity]
  marker = error_frac
  max_h_level = 5
  [Indicators]
    [u0_jump]
      type = GradientJumpIndicator
      variable = u0
      scale_by_flux_faces = false
    []
  []
  [Markers]
    [error_frac]
      type = ErrorFractionMarker
      coarsen = 0.15
      indicator = u0_jump
      refine = 0.7
    []
  []
[]

[Variables]
  [u0]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    block = 0
    initial_condition = 0
  []

  [u1]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    block = 1
    initial_condition = 0
  []
[]

[FVKernels]
  [time0]
    type = FVTimeKernel
    variable = u0
  []

  [diff0]
    type = FVDiffusion
    variable = u0
    coeff = 1
    block = 0
  []

  [time1]
    type = FVTimeKernel
    variable = u1
  []

  [diff1]
    type = FVDiffusion
    variable = u1
    coeff = 1
    block = 1
  []
[]

[FVInterfaceKernels]
  [diffusion]
    type = FVDiffusionInterface
    variable1 = u0
    variable2 = u1
    boundary = interface
    subdomain1 = 0
    subdomain2 = 1
    coeff1 = 1
    coeff2 = 1
  []
[]

[FVBCs]
  [left] # arbitrary user-chosen name
    type = FVDirichletBC
    variable = u0
    boundary = 'left' # This must match a named boundary in the mesh file
    value = 1
  []

  [right] # arbitrary user-chosen name
    type = FVNeumannBC
    variable = u1
    boundary = 'right' # This must match a named boundary in the mesh file
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = 'Newton'

  end_time = 0.5
  dt = 0.1
[]

[VectorPostprocessors]
  [samples]
    type = LineValueSampler
    variable = u0
    # Avoiding element faces
    start_point = '0.0001 1e-6 0'
    end_point = '0.999999 1e-6 0'
    num_points = 10
    sort_by = 'x'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
