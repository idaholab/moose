[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [top]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.25
    xmax = 0.25
    ymin = -0.25
    ymax = 0.25
    zmin = -0.25
    zmax = 0.25
    elem_type = TET10
    boundary_name_prefix = top
  []
  [top_id]
    type = SubdomainIDGenerator
    input = top
    subdomain_id = 1
  []
  [bottom]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.3
    zmax = -0.25
    elem_type = TET10
    boundary_name_prefix = bottom
    boundary_id_offset = 10
  []
  [bottom_id]
    type = SubdomainIDGenerator
    input = bottom
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'top_id bottom_id'
  []
  [Partitioner]
    type = GridPartitioner
    # Three slabs across the interface put the secondary faces around some contact nodes on
    # processes other than the node owner
    nx = 3
  []
  [curve]
    type = ParsedNodeTransformGenerator
    input = combined
    x_function = x
    y_function = y
    z_function = 'z + 0.04 * x * x'
  []
[]

[Variables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
  []
[]

[Kernels]
  [disp_x]
    type = Diffusion
    variable = disp_x
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
  []
  [disp_z]
    type = Diffusion
    variable = disp_z
  []
[]

# GeneratedMeshGenerator in 3D: 'back' is the zmin face and 'front' the zmax face
[Contact]
  [mortar]
    primary = bottom_front
    secondary = top_back
    formulation = mortar_penalty
    model = frictionless
    penalty = 1e2
  []
[]

[Problem]
  # The test fails if contact assembles a Jacobian entry outside the preallocated sparsity pattern
  error_on_jacobian_nonzero_reallocation = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # Assembling the Jacobian once is all this test checks, so a single forced Newton step suffices
  # and the tolerance is loose enough that the solve converges after it
  nl_forced_its = 1
  nl_max_its = 1
  nl_abs_tol = 1e-3
[]
