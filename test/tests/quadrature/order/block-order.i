[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 2
  []

  [bottom]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [top]
    type = SubdomainBoundingBoxGenerator
    input = bottom
    block_id = 2
    bottom_left = '0 1 0'
    top_right = '1 2 0'
  []
  [middle]
    type = SideSetsBetweenSubdomainsGenerator
    input = top
    primary_block = 1
    paired_block = 2
    new_boundary = middle
  []
[]

[Postprocessors]
  [block1_qps]
    type = NumElemQPs
    block = 1
  []
  [block2_qps]
    type = NumElemQPs
    block = 2
  []
  [top_side_qps]
    type = NumSideQPs
    boundary = top
  []
  [bottom_side_qps]
    type = NumSideQPs
    boundary = bottom
  []
  [middle_side_qps]
    type = NumSideQPs
    boundary = middle
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
  [Quadrature]
    custom_blocks = '1 2'
    custom_orders = 'first second'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = false
  csv = true
[]
