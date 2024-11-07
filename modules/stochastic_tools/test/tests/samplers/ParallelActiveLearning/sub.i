left_bc = 2.8833503
right_bc = 2.5709298

param1 = '${fparse left_bc}'
param2 = '${fparse right_bc}'

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  xmin = 0
  ymax = 1
  ymin = 0
  nx = 10
  ny = 10
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
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = ${param1}
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = ${param2}
  []
[]

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  console = 'false'
[]
