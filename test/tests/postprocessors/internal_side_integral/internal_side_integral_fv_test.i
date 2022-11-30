[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 4
  ymin = 0
  ymax = 1
[]

[Variables]
  active = 'u'

  [./u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[FVKernels]
  active = 'diff'

  [./diff]
    type = FVDiffusion
    variable = u
    coeff = '1'
  [../]
[]

[FVBCs]
  active = 'left right'

  [./left]
    type = FVDirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = FVDirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./integral]
    type = InternalSideIntegralVariablePostprocessor
    variable = u
  [../]
[]

[Outputs]
  file_base = fv_out
  exodus = true
[]
