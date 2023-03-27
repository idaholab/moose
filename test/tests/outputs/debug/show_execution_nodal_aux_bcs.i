[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [sub]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = 'gen'
    block_id = '1'
  []
[]

[Debug]
  show_execution_order = ALWAYS
[]

[Variables]
  [u]
    block = '0 1'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rxn]
    type = Reaction
    variable = u
  []
[]

[AuxVariables]
  [v]
  []
[]

[BCs]
  [setting]
    type = NeumannBC
    variable = u
    boundary = 'top'
    value = '4'
  []
[]

[Executioner]
  type = Steady
[]

[Dampers]
  inactive = 'limit_u'
  [limit_u]
    type = BoundingValueNodalDamper
    variable = u
    max_value = 1.5
    min_value = -20
  []
[]
