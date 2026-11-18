[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[AuxVariables]
  [elem_u]
    family = MONOMIAL
    order = FIRST
  []
  [elem_v]
    family = MONOMIAL
    order = FIRST
  []
  [node_u]
  []
  [node_v]
  []
  [dep]
  []
[]

[AuxKernels]
  # Couples the second variable of the multi-variable kernel below, so it must be computed after
  # that kernel even though it is listed first
  [dep]
    type = KokkosParsedAux
    variable = dep
    coupled_variables = 'node_v'
    expression = 'node_v'
  []
  [elem]
    type = KokkosComponentAux
    variables = 'elem_u elem_v'
  []
  [node]
    type = KokkosComponentAux
    variables = 'node_u node_v'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
