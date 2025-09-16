
# nx = 1024
nx = 16
nx_half = '${fparse nx/2}'
x0 = 0.5
x0_double = '${fparse 2*x0}'

[Problem]
  solve = false
  # boundary_restricted_node_integrity_check = false
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${x0} ${x0}'
    dy = '${x0_double}'
    ix = '${nx_half} ${nx_half}'
    iy = '${nx}'
    subdomain_id = '1 2'
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
