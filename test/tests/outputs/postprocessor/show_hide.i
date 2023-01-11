# Having 2 postprocessors, putting one into hide list and the other one into show list
# We should only see the PPS that is in the show list in the output.

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

[Functions]
  [./bc_fn]
    type = ParsedFunction
    expression = x
  [../]
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
  [./all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 3'
    function = bc_fn
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
    hide = 'elem_12'
  [../]
  [./out]
    type = CSV
    show = 'elem_56'
    hide = 'elem_12'
  [../]
[]
