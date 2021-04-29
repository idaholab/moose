[Mesh]
  [file]
    type = FileMeshGenerator
    file = flow_test.e
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    new_block = 11
    new_block = "secondary"
    sidesets = '1'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    new_block = 12
    new_block = "primary"
    sidesets = '2'
  []
[]

[Variables]
  [u]
    block = 'bottom middle top'
  []
  [lm]
    block = 'secondary'
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
    secondary_variable = u
    primary_boundary = top
    secondary_boundary = bottom
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
