# Two Physics on disconnected blocks share the finite volume variable 'u'. Block 2, between them,
# has no Physics.
right_block = 3
right_scaling = 1

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
  [Test]
    [SharedVariable]
      [left]
        block = 0
        fv = true
      []
      [right]
        block = ${right_block}
        fv = true
        scaling = ${right_scaling}
      []
    []
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 0
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
