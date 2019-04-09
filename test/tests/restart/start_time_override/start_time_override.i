[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Problem]
  restart_file_base = transient_out_cp/LATEST
  skip_additional_restart_data = true
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [bodyforce]
    type = BodyForce
    variable = u
    value = 10.0
  []

  [ie]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 10
  []
[]

[Postprocessors]
  [u_norm]
    type = ElementL2Norm
    variable = u
  []
[]

[Executioner]
  type = Transient

  # Start time can be set explicitly here or be picked up from the restart file
  num_steps = 5
  dt = 0.1
[]

[Outputs]
  csv = true
[]
