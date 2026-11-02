# Two Physics on adjacent blocks share the variables 'u' and 'v'
order = FIRST

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
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
  [Test]
    [SharedVariable]
      [left]
        block = 1
      []
      [right]
        block = 2
        order = ${order}
      []
    []
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
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
  [u_blocks]
    type = ElementExtremeValue
    variable = u
    value_type = max
    block = '1 2'
  []
[]

[Outputs]
  csv = true
[]
