# Only the density adjustment factor is requested, so only its (user-created)
# elemental aux variable needs to exist. With displacement_x = 0.1*x the factor is
# V_orig/V_new = 1/1.1 per element.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[AuxVariables]
  [jratio]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpression
    block = 0
    displacement_x = '0.1*x'
    density_factor_variable = 'jratio'
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
