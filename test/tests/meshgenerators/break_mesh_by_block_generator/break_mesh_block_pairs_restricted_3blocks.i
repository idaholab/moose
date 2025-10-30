[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1 1 1'
    subdomain_id = '0 0 0 0 1 2 2 1 1 1 1 1'
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    split_interface = true
    add_interface_on_two_sides = false
  []
[]

[AuxVariables]
  [proc]
    [AuxKernel]
      type = ProcessorIDAux
      execute_on = initial
    []
  []
  [proc_elem]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ProcessorIDAux
      execute_on = initial
    []
  []
[]

