# Two diffusion Physics on adjacent blocks share the variable 'u'. Without sharing, the second
# Physics would add 'u' again, or error because the existing 'u' does not cover its blocks.
right_order = FIRST

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = left
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[Physics]
  [Diffusion]
    [ContinuousGalerkin]
      [left]
        block = 1
        diffusivity_functor = 1
        dirichlet_boundaries = 'left'
        boundary_values = '1'
      []
      [right]
        block = 2
        variable_order = ${right_order}
        diffusivity_functor = 1
        dirichlet_boundaries = 'right'
        boundary_values = '0'
      []
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [u_mid]
    type = PointValue
    variable = u
    point = '0.5 0 0'
  []
[]

[Outputs]
  csv = true
[]
