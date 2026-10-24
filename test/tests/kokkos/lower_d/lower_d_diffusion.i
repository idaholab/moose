[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [lower_d]
    type = LowerDBlockFromSidesetGenerator
    input = square
    sidesets = 'bottom'
    new_block_id = 10
    new_block_name = 'bottom_lower'
  []
[]

[Variables]
  [u]
    block = 0
  []
  [v]
    block = bottom_lower
  []
[]

[Kernels]
  [diff_u]
    type = KokkosDiffusion
    variable = u
    block = 0
  []
  [diff_v]
    type = KokkosDiffusion
    variable = v
    block = bottom_lower
  []
  [source_v]
    type = KokkosBodyForce
    variable = v
    value = 1
    block = bottom_lower
  []
[]

[BCs]
  [u_left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [u_right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [v_ends]
    type = KokkosDirichletBC
    variable = v
    boundary = 'left right'
    value = 0
  []
[]

[Postprocessors]
  [u_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  []
  [v_integral]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = bottom_lower
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
