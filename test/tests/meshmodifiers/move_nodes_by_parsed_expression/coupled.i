# Coupled nodal variable check: the displacement uses the nodal variable d (= y),
# so the x-coordinate becomes original_x + 0.2*y. Exodus stores a single
# node-coordinate array, so output only at FINAL to capture the moved positions.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[AuxVariables]
  [d]
  []
[]

[AuxKernels]
  [set_d]
    type = FunctionAux
    variable = d
    function = 'y'
    execute_on = 'INITIAL'
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpression
    block = 0
    displacement_x = '0.2*d'
    coupled_variables = 'd'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
