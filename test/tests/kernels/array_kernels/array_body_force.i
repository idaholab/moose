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
  [source]
    type = ArrayBodyForce
    variable = u
    function = '1 x'
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = 'left right bottom top'
    values = '0 0'
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
