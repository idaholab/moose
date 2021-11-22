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
    initial_condition = 7
  []
[]

[Kernels]
[]

[FVKernels]
  [./time]
    type = FVTimeKernel
    variable = v
  [../]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '.2'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  scheme = newmark-beta
  num_steps = 20
  dt = 0.1
[]

[Postprocessors]
  [vdotdot]
    type = ADElementAverageSecondTimeDerivative
    variable = v
  []
[]

[Outputs]
  csv = true
[]
