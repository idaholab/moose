[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
    scaling = '.9 .9'
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = 1
    values = '0 0'
  []

  [right]
    type = ArrayDirichletBC
    variable = u
    boundary = 2
    values = '1 2'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
