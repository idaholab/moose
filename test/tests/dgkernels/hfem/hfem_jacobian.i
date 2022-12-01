[Mesh]
  [square]
    type = GeneratedMeshGenerator
    elem_type = QUAD9 # SIDE_HIERARCHIC needs side nodes
    nx = 3
    ny = 3
    dim = 2
  []
[]

[Variables]
  [u]
    order = FOURTH
    family = MONOMIAL
  []
  [lambda]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = '1'
  []
  [source]
    type = BodyForce
    variable = u
    value = '1'
  []
[]

[DGKernels]
  [testjumps]
    type = HFEMTestJump
    variable = u
    side_variable = lambda
  []
  [trialjumps]
    type = HFEMTrialJump
    variable = lambda
    interior_variable = u
  []
[]

[BCs]
  [u_robin]
    type = VacuumBC
    boundary = 'left right top bottom'
    variable = u
  []
  [lambda_D_unused]
    type = PenaltyDirichletBC
    boundary = 'left right top bottom'
    variable = lambda
    penalty = 1
  []
[]

[Postprocessors]
  [intu]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  []
  [lambdanorm]
    type = ElementSidesL2Norm
    variable = lambda
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       basic                 mumps'
[]
