# Drift-free check: the modifier runs every TIMESTEP_BEGIN with a time-dependent
# displacement. Because displacement is evaluated relative to each node's original
# position, after two steps the x-coordinate is original + 0.05*2 = original + 0.10
# (an accumulating implementation would instead give 0.05 + 0.10 = 0.15). Exodus
# stores a single node-coordinate array, so output only at FINAL to capture the
# moved positions at the last step.
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

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpressionModifier
    block = 0
    displacement_x = '0.05*t'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
