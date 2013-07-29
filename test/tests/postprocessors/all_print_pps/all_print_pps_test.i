[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5

  xmin = 0
  xmax = 2

  ymin = 0
  ymax = 2
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

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

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Postprocessors]
  [./nodes]
    type = PrintNumNodes
    variable = u
  [../]

  [./elements]
    type = NumElems
    variable = u
  [../]

  [./dofs]
    type = PrintDOFs
    variable = u
  [../]

  [./residuals]
    type = NumResidualEvaluations
    variable = u
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = false
  postprocessor_csv = true
  output_initial = true
  perf_log = true
[]
