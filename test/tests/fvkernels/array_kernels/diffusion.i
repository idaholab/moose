[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
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
[]

[FVBCs]
  [left]
    type = FVArrayDirichletBC
    variable = u
    boundary = 0
    value = '0 0'
  []

  [right]
    type = FVArrayDirichletBC
    variable = u
    boundary = 1
    value = '1 2'
  []
[]

[Materials]
  [dc]
    type = ADGenericConstantArray
    prop_name = dc
    prop_value = '1 1'
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
