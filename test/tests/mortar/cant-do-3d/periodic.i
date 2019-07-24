[Mesh]
  file = flow_test.e
[]

[MeshModifiers]
  [slave]
    type = LowerDBlockFromSideset
    new_block_id = 11
    new_block_name = "slave"
    sidesets = '1'
  []
  [master]
    type = LowerDBlockFromSideset
    new_block_id = 12
    new_block_name = "master"
    sidesets = '2'
  []
[]

[Variables]
  [u]
    block = 'bottom middle top'
  []
  [lm]
    block = 'slave'
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
    block = 'bottom middle top'
  []
  [force]
    type = BodyForce
    variable = u
    block = 'bottom middle top'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 'around'
  []
[]

[Constraints]
  [ev]
    type = EqualValueConstraint
    variable = lm
    slave_variable = u
    master_boundary = top
    slave_boundary = bottom
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
