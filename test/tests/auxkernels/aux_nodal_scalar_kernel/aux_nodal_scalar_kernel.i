[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 0
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2
  [../]
[]

[AuxVariables]
  [./bc_sum]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxScalarKernels]
  [./sk]
    type = SumNodalValuesAux
    variable = bc_sum
    nodes = '0 10'
    sum_var = u
  [../]
[]

[Postprocessors]
  [./sum]
    type = ScalarVariable
    variable = bc_sum
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  hide = bc_sum
[]
