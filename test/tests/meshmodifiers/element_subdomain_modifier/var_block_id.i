[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 4
    ymax = 4
    nx = 8
    ny = 8
  []
  [subdomain_id]
    input = gen
    type = SubdomainPerElementGenerator
    subdomain_ids = '0 1 2 3 4 5 6 7
    0 1 2 3 4 5 6 7
    0 1 2 3 4 5 6 7
    0 1 2 3 4 5 6 7
    8 9 10 11 12 13 14 15
    8 9 10 11 12 13 14 15
    8 9 10 11 12 13 14 15
    8 9 10 11 12 13 14 15'
  []
[]

[AuxVariables]
  [u]
  []
  [block_id]
  []
[]

[AuxKernels]
  [u0]
    type = ConstantAux
    variable = u
    value = 1
    block = '0 4 8 12'
  []
  [u1]
    type = ConstantAux
    variable = u
    value = 10
    block = '1 5 9 13'
  []
  [u2]
    type = ConstantAux
    variable = u
    value = 100
    block = '2 6 10 14'
  []
  [u3]
    type = ConstantAux
    variable = u
    value = 1000
    block = '3 7 11 15'
  []
  [block_id]
    type = FunctionAux
    variable = block_id
    function = fcn
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[MeshModifiers]
  [assign_block_id]
    type = VariableValueElementSubdomainModifier
    coupled_var = 'block_id'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Functions]
  [fcn]
    type = ParsedFunction
    expression = 'x+y+t'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
