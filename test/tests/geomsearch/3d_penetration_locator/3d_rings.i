[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = rings_facing.e
  []
  allow_renumbering = false
[]

[AuxVariables]
  [penet]
    family = MONOMIAL
    order = CONSTANT
    block = structure
  []
[]

[AuxKernels]
  [penet]
    type = PenetrationAux
    variable = penet
    boundary = 'bottom_structure'
    paired_boundary = 'top_heat_source'
  []
[]

[VectorPostprocessors]
  [values]
    type = ElementValueSampler
    variable = penet
    sort_by = 'id'
    block = 'structure'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
[]
