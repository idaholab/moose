left_bc = 0.13508909593042528
right_bc = -1.5530467809139854
mesh1 = 1

param1 = '${fparse left_bc}'
param2 = '${fparse right_bc}'
param3 = '${fparse mesh1}'

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = ${param3}
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
    value = ${param1} # Actual = 0.15
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = ${param2} # Actual = -1.5
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

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Outputs]
  console = 'false'
[]
