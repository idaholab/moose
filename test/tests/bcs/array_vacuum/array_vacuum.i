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
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[BCs]
  [left]
    type = ArrayVacuumBC
    variable = u
    boundary = 1
    alpha = '1 1.2'
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
  [rc]
    type = GenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
