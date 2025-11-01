[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[KokkosFunctions]
  [constant]
    type = KokkosConstantFunction
    value = 2
  []
[]

[KokkosKernels]
  [diff]
    type = KokkosFuncCoefDiffusion
    variable = u
    coef = constant
  []
  [time]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[KokkosBCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosNeumannBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
