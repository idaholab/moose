# The WebServerControl will change the value of BCs/left/value.
# We run this test in parallel so that it tests the broadcast
# of the value that is being received by the webserver on rank 0

[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 10
  ny = 10
[]

[Variables/u]
[]

[Kernels]
  [dot]
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
    value = 0 # the control will change this value
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Controls/web_server]
  type = WebServerControl
  port = 8000 # will get overridden by the script to find an available port
  execute_on = 'TIMESTEP_BEGIN'
[]

[Executioner]
  type = Transient
  num_steps = 4
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
[]

[Outputs]
  exodus = true
[]
