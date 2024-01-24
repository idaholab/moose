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

[Physics]
  [Diffusion]
    [FiniteVolume]
      [diff]
        source_functor = 2

        # Test all the ways of setting the boundary conditions
        neumann_boundaries = 'left_to_0 right_to_0 top_to_0 bottom_to_0'
        boundary_fluxes = '1 flux_pp flux_function flux_variable'
        dirichlet_boundaries = 'left_to_1 right_to_1 top_to_1 bottom_to_1'
        boundary_values = '2 value_pp value_function value_variable'
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

# To test setting up a boundary condition with a postprocessor
[Postprocessors]
  [flux_pp]
    type = Receiver
    default = 1
    outputs = 'none'
  []
  [value_pp]
    type = Receiver
    default = 2
    outputs = 'none'
  []
[]

# To test setting up a boundary condition with a function
[Functions]
  [flux_function]
    type = ConstantFunction
    value = 1
  []
  [value_function]
    type = ConstantFunction
    value = 2
  []
[]

# To test setting up a boundary condition with a variable
[AuxVariables]
  [flux_variable]
    type = MooseVariableFVReal
    initial_condition = 1
  []
  [value_variable]
    type = MooseVariableFVReal
    initial_condition = 2
  []
[]

# Form output for testing
[VectorPostprocessors]
  [sample]
    type = ElementValueSampler
    variable = 'u'
    sort_by = 'id'
  []
[]

[Outputs]
  csv = true
[]
