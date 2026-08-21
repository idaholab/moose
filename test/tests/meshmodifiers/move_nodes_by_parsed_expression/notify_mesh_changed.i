# With notify_mesh_changed = true the modifier notifies the problem that the mesh
# changed after moving nodes each timestep, so Exodus re-outputs the moved mesh
# (a new file sequence segment per step). The final segment holds the mesh at
# t = 2, displaced by 0.05*t = 0.10 in x. This also confirms the notification does
# not recurse (the modifier does not respond to meshChanged).
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
    type = MoveNodesByParsedExpression
    block = 0
    displacement_x = '0.05*t'
    notify_mesh_changed = true
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Outputs]
  exodus = true
[]
