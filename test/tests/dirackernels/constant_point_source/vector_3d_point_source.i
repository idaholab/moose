[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE_VEC
  []
[]

[Kernels]
  [diff]
    type = VectorDiffusion
    variable = u
  []
[]

[DiracKernels]
  [point_source1]
    type = VectorConstantPointSource
    variable = u
    values = '0.1 0.1 0.1'
    point = '0.2 0.3 0.0'
  []
  [point_source2]
    type = VectorConstantPointSource
    variable = u
    values = '-0.1 -0.1 -0.1'
    point = '0.2 0.8 0.0'
  []
  [point_source3]
    type = VectorConstantPointSource
    variable = u
    values = '-1.0 -1.0 -1.0'
    point = '0.8 0.5 0.8'
  []
[]

[BCs]
  [left]
    type = VectorDirichletBC
    variable = u
    boundary = left
    values = '0 0 0'
  []
  [right]
    type = VectorDirichletBC
    variable = u
    boundary = right
    values = '1 1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

[]

[Outputs]
  file_base = vector_3d_out
  exodus = true
[]
