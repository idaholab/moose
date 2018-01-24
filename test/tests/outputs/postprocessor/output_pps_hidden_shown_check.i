# Computing two postprocessors and specifying one of them both in the
# show list and the hide list, which should throw an error message.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
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
  [./lr_u]
    type = DirichletBC
    variable = u
    boundary = '1 3'
    value = 1
  [../]
[]

[Postprocessors]
  [./elem_56]
    type = ElementalVariableValue
    variable = u
    elementid = 56
  [../]

  [./elem_12]
    type = ElementalVariableValue
    variable = u
    elementid = 12
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  [./console]
    type = Console
    show = 'elem_56'
    hide = 'elem_56'
  [../]
[]
