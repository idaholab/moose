# NOTE: This file is used within the documentation, so please do not change names within the file
# without checking that associated documentation is not affected, see syntax/Postprocessors/index.md.
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

[AuxVariables]
  [aux]
    initial_condition = 5
  []
[]

[Kernels]
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
    value = 0
  []
  [right]
    type = PostprocessorNeumannBC
    variable = u
    boundary = right
    postprocessor = right_pp
  []
[]

[Postprocessors]
  [right_pp]
    type = PointValue
    point = '0.5 0.5 0'
    variable = aux
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
