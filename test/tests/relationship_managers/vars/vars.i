# illustrates what i think is a bug in the RelationshipManager
#
# u0 = 1 at nodes where u can be evaluated by rank=0 (and u0 = -1 otherwise)
# u1 = 1 at nodes where u can be evaluated by rank=1 (and u1 = -1 otherwise)
#
# Since the ElemSideNeighborLayersVarTester UserObjects have element_side_neighbor_layers = 2,
# when using mpirun -np 2 blah blah
# u0 and u1 should be = 1 at all nodes save for 1 (when the mesh has 6 elements)
#
# Running this file with or without --distributed-mesh does not produce the expected result
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 6
[]

[GlobalParams]
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Kernels]
  [./u]
    type = Diffusion
    variable = u
  [../]
[]

[AuxVariables]
  [u0]
  []
  [u1]
  []
  [proc]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [u0]
    type = ElemSideNeighborLayersVarTesterAux
    variable = u0
    var_tester_uo = var_tester0
  []
  [u1]
    type = ElemSideNeighborLayersVarTesterAux
    variable = u1
    var_tester_uo = var_tester1
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
  []
[]

[UserObjects]
  [var_tester0]
    type = ElemSideNeighborLayersVarTester
    element_side_neighbor_layers = 2
    rank = 0
    u = u
  []
  [var_tester1]
    type = ElemSideNeighborLayersVarTester
    element_side_neighbor_layers = 2
    rank = 1
    u = u
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1
[]

[Outputs]
  exodus = true
[]
