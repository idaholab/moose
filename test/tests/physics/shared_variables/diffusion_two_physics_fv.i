# Two finite volume diffusion Physics on blocks that do not touch share the variable 'u'. Block 2,
# between them, has no Physics.
right_block = 3

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
  [middle]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 2
    bottom_left = '0.4 0 0'
    top_right = '0.6 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = middle
    block_id = 3
    bottom_left = '0.6 0 0'
    top_right = '1 1 0'
  []
[]

[Physics]
  [Diffusion]
    [FiniteVolume]
      [left]
        block = 0
        diffusivity_functor = 1
        dirichlet_boundaries = 'left'
        boundary_values = '1'
      []
      [right]
        block = ${right_block}
        diffusivity_functor = 1
        dirichlet_boundaries = 'right'
        boundary_values = '0'
      []
    []
  []
[]

[Problem]
  # Block 2 has no Physics, hence no kernels
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [u_left]
    type = ElementAverageValue
    variable = u
    block = 0
  []
  [u_right]
    type = ElementAverageValue
    variable = u
    block = ${right_block}
  []
[]

[Outputs]
  csv = true
[]
