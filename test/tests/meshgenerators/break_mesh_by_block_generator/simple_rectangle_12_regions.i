# nx = 1024

[Problem]
  solve = false
  boundary_restricted_node_integrity_check = false
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.2 0.3 0.5'
    dy = '0.3 0.3 0.3 0.1'
    ix = '6 9 15'
    iy = '9 9 9 3'

    subdomain_id = '1 2 3 4 5 6 7 8 9 10 11 12'
  []

  [break]
    type = BreakMeshByBlockGenerator
    input = gen
    split_interface = true
    add_interface_on_two_sides = true
  []

  parallel_type = distributed
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
