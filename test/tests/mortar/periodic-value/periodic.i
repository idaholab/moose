[Mesh]
  [file]
    type = FileMeshGenerator
    file = square.msh
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 11
    new_block_name = "secondary"
    sidesets = '101'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 12
    new_block_name = "primary"
    sidesets = '103'
  []
[]

[Variables]
  [u]
    order = SECOND
    block = 'domain'
  []
  [lm]
    block = 'secondary'
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
    secondary_variable = u
    primary_boundary = 103
    secondary_boundary = 101
    primary_subdomain = 12
    secondary_subdomain = 11
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
