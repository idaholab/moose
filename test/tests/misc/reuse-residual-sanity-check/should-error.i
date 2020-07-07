[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = BodyForce
    postprocessor = resid_counter
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
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -snes_max_it -ksp_max_it'
  petsc_options_value = 'hypre    1            5'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [resid_counter]
    type = NumResidualEvaluations
    execute_on = 'linear nonlinear'
  []
[]
