[Problem]
  solve = false
  boundary_restricted_node_integrity_check = false
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
    split_interface = true
    add_interface_on_two_sides = true
  []
[]

[Outputs]
  exodus = true
[]

[Variables]
  [diffused]
    order = FIRST
  []
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [proc]
  []
  [proc_elem]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  []
  [proc_elem]
    type = ProcessorIDAux
    variable = proc_elem
    execute_on = initial
  []
[]
