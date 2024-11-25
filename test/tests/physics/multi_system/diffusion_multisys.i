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
    [ContinuousGalerkin]
      [diff_1]
        source_functor = 2
        system_names = 'sys1'

        dirichlet_boundaries = 'left_to_1 right_to_1 top_to_1 bottom_to_1'
        boundary_values = '2 3 4 5'
      []
      [diff_2]
        source_functor = 2
        system_names = 'sys2'
        variable_name = 'v'

        dirichlet_boundaries = 'left_to_1 right_to_1 top_to_1 bottom_to_1'
        boundary_values = '2 3 4 5'
      []
    []
  []
[]

[Problem]
  nl_sys_names = 'sys1 sys2'
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
