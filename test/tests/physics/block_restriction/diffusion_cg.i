[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 2'
    dy = '2 1'
    ix = '2 3'
    iy = '3 2'
    subdomain_id = '0 1
                    1 0'
  []
  [split_boundaries]
    type = BreakBoundaryOnSubdomainGenerator
    input = cmg
  []
  allow_renumbering = false
[]

# We need to add these items before the Physics to trigger the
# Physics behavior of erroring / skipping the objects if incompatible/compatible objects
# already exist
[Variables]
  [u]
  []
[]

[ICs]
  [extern]
    type = FunctionIC
    function = 0
    variable = u
  []
[]

[Kernels]
  active = ''
  [extern]
    type = TimeDerivative
    variable = u
  []
[]

[Physics]
  [Diffusion]
    [ContinuousGalerkin]
      [diff]
        source_functor = 2
        diffusivity_matprop = '1'

        # Test all the ways of setting the boundary conditions
        neumann_boundaries = 'left_to_0 right_to_0 top_to_0 bottom_to_0'
        boundary_fluxes = '1 1 1 1'
        dirichlet_boundaries = 'left_to_1 right_to_1 top_to_1 bottom_to_1'
        boundary_values = '2 2 2 2'
      []
    []
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 10
  # Output the setup
  verbose = true
[]

# Form output for testing
[VectorPostprocessors]
  [sample]
    type = NodalValueSampler
    variable = 'u'
    sort_by = 'id'
  []
[]

[Outputs]
  csv = true
[]
