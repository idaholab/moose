[Mesh]
  type = FileMesh
  file = constraints.e
  # NearestNodeLocator, which is needed by TiedValueConstraint,
  # only works with ReplicatedMesh currently
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
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
    boundary = 4
    value = 1
  []
[]

[Constraints]
  [complete]
    type = TiedValueConstraint
    variable = u
    secondary = 2
    primary = 3
    primary_variable = u
  []
  [lower]
    type = TiedValueConstraint
    variable = u
    secondary = inside_right_lower
    primary = inside_left_lower
    primary_variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [test_time_type]
    type = TestVectorType
    system = nl
    vector = TIME
    vector_type = ghosted
  []
  [test_nontime_type]
    type = TestVectorType
    system = nl
    vector = NONTIME
    vector_type = ghosted
  []
[]
