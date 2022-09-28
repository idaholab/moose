[Mesh]
  [file]
    type = FileMeshGenerator
    file = flow_test.e
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 11
    new_block_name = "secondary"
    sidesets = '1'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 12
    new_block_name = "primary"
    sidesets = '2'
  []
[]

[Variables]
  [u]
    block = 'bottom middle top'
  []
  [lm]
    block = 'secondary'
    use_dual = true
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
    block = 'bottom middle'
    function = 'x - y'
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
    delta = 0.1
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
  nl_rel_tol = 1e-12
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
