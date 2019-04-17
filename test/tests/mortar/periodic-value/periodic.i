[Mesh]
  file = square.msh
[]

[MeshModifiers]
  [slave]
    type = LowerDBlockFromSideset
    new_block_id = 11
    new_block_name = "slave"
    sidesets = '101'
  []
  [master]
    type = LowerDBlockFromSideset
    new_block_id = 12
    new_block_name = "master"
    sidesets = '103'
  []
[]

[Variables]
  [u]
    order = SECOND
    block = 'domain'
  []
  [lm]
    block = 'slave'
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
    block = 'domain'
  []
  [force]
    type = BodyForce
    variable = u
    block = 'domain'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 'left'
  []
[]

[Constraints]
  [ev]
    type = EqualValueConstraint
    variable = lm
    master_variable = u
    master_boundary = 103
    slave_boundary = 101
    master_subdomain = 12
    slave_subdomain = 11
    periodic = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
