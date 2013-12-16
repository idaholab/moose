[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = SECOND
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./a]
    order = SECOND
    family = SCALAR
  [../]
[]

[ICs]
  [./v_ic]
    type = ScalarComponentIC
    variable = 'v'
    values = '1 2'
  [../]

  [./a_ic]
    type = ScalarComponentIC
    variable = 'a'
    values = '4 5'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[ScalarKernels]
  [./ask]
    type = AlphaCED
    variable = v
    value = 100
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Postprocessors]
  [./v1]
    type = ScalarVariable
    variable = v
    component = 0
  [../]
  [./v2]
    type = ScalarVariable
    variable = v
    component = 1
  [../]

  [./a1]
    type = ScalarVariable
    variable = a
    component = 0
  [../]
  [./a2]
    type = ScalarVariable
    variable = a
    component = 1
  [../]
[]


[Executioner]
  type = Steady
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
