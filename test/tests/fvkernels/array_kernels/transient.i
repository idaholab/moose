[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = '7 8'
    components = 2
  []
[]

[Kernels]
[]

[FVKernels]
  [./time]
    type = FVArrayTimeKernel
    variable = v
  [../]
  [diff]
    type = FVArrayDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVArrayDirichletBC
    variable = v
    boundary = left
    value = '7 11'
  []
  [right]
    type = FVArrayDirichletBC
    variable = v
    boundary = right
    value = '42 42'
  []
[]

[Materials]
  [diff]
    type = ADGenericConstantArray
    prop_name = 'coeff'
    prop_value = '.2 .3'
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  num_steps = 20
  dt = 0.02
[]

[Outputs]
  exodus = true
[]
