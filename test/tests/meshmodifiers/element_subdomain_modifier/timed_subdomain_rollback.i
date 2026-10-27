# The initial dt=1 trial crosses t=0.75 and moves every element from block 0 to block 1.
# Terminator rejects that trial and cuts dt to 0.5. The retry must restore the elements to block 0.
# Without rollback, the accepted mesh incorrectly remains in block 1.
[Mesh]
  add_subdomain_ids = '1'

  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmin = 0
    xmax = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[MeshModifiers]
  [move_0_to_1]
    type = TimedSubdomainModifier
    times = '0.75'
    blocks_from = '0'
    blocks_to = '1'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[UserObjects]
  [reject_large_dt]
    type = Terminator
    expression = 'dt > 0.5'
    fail_mode = SOFT
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
