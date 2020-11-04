[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    components = 2
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVArrayDiffusion
    variable = u
    coeff = dc
  []
  [reaction]
    type = FVArrayReaction
    variable = u
  []
[]

[FVBCs]
  [left]
    type = FVArrayDirichletBC
    variable = u
    boundary = left
    value = '0 0'
  []
  [right]
    type = FVArrayDirichletBC
    variable = u
    boundary = right
    value = '1 2'
  []
[]

[Materials]
  [dc]
    type = ADGenericConstantArray
    prop_name = dc
    prop_value = '.1 .1'
  []
[]

[Problem]
  kernel_coverage_check = off
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
