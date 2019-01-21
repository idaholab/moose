[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    initial_condition = 10
  []
[]

[AuxVariables]
  [v]
    initial_condition = 20
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 10
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 20
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [final]
    type = Exodus
    execute_on = 'FINAL'
    execute_input_on = 'NONE' # This is needed to avoid problems with creating a file w/o data during --recover testing
  []
[]
