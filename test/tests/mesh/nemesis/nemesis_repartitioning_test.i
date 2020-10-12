[Mesh]
  file = cylinder/cylinder.e
  nemesis = true
  # leaving skip_partitioning off lets us exodiff against a gold
  # standard generated with default libMesh settings
  # skip_partitioning = true
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
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
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-14
  [Adaptivity]
    steps = 1
    refine_fraction = 0.1
    coarsen_fraction = 0.1
    max_h_level = 2
  []
[]

[Postprocessors]
  [sum_sides]
    type = StatVector
    stat = sum
    object = nl_wb_element
    vector = num_partition_sides
  []
  [min_elems]
    type = StatVector
    stat = min
    object = nl_wb_element
    vector = num_elems
  []
  [max_elems]
    type = StatVector
    stat = max
    object = nl_wb_element
    vector = num_elems
  []
[]

[VectorPostprocessors]
  [nl_wb_element]
    type = WorkBalance
    execute_on = initial
    system = nl
    balances = 'num_elems num_partition_sides'
    outputs = none
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
