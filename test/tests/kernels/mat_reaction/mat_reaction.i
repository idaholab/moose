AD = ''

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Variables]
  [u]
  []
[]

[Materials]
  [ad_prop]
    type = ${AD}ParsedMaterial
    expression = '-log(3)*log(3)'
    property_name = rxn_prop
  []
[]

[Kernels]
  [diff]
    type = ${AD}Diffusion
    variable = u
  []
  [reaction]
    type = ${AD}MatReaction
    variable = u
    reaction_rate = rxn_prop
  []
[]

[BCs]
  [left]
    type = ${AD}DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = ${AD}DirichletBC
    variable = u
    boundary = right
    value = 3
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  file_base = ${AD}mat_reaction_out
[]
