[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
  # This test will not work in parallel with DistributedMesh enabled
  # due to a bug in PeriodicBCs.
  parallel_type = replicated
[]

[Functions]
  [./bc_fn]
    type = ParsedGradFunction
    value = -sin(pi*x)*sin(pi*y)
    grad_x = -pi*cos(pi*x)*sin(pi*y)
    grad_y = -pi*sin(pi*x)*cos(pi*y)
  [../]
  [./forcing_fn]
    type = ParsedFunction
    expression = -2*pi*pi*sin(pi*x)*sin(pi*y)-sin(pi*x)*sin(pi*y)
  [../]
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./reaction]
    type = Reaction
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionPenaltyDirichletBC
    variable = u
    boundary = 'bottom right top left'
    function = bc_fn
    penalty = 1e10
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]
  [./h]
    type = AverageElementSize
  [../]
  [./L2error]
    type = ElementL2Error
    variable = u
    function = bc_fn
  [../]
  [./H1error]
    type = ElementH1Error
    variable = u
    function = bc_fn
  [../]
  [./H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = bc_fn
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  # We use higher-order quadrature to ensure that the forcing function
  # is integrated accurately.
  [./Quadrature]
    order=ELEVENTH
  [../]
[]

[Adaptivity]
  steps = 2
  marker = uniform
  [./Markers]
    [./uniform]
      type = UniformMarker
      mark = refine
    [../]
  [../]
[]


[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  print_mesh_changed_info = true
[]
