[Mesh]
  file = cylinder/cylinder.e
  nemesis = true
  # leaving skip_partitioning off lets us exodiff against a gold
  # standard generated with default libMesh settings
  # skip_partitioning = true
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Postprocessors]
  [./elem_avg]
    type = ElementAverageValue
    variable = u
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  [./Adaptivity]
    steps = 1
    refine_fraction = 0.1
    coarsen_fraction = 0.1
    max_h_level = 2
  [../]

  nl_rel_tol = 1e-6
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
  file_base = repartitioned
  nemesis = true
[]

[VectorPostprocessors]
  [./nl_wb_element]
    type = WorkBalance
    execute_on = initial
    system = nl
    outputs  = none
    balances = 'num_elems num_nodes'
  []

  [./bl_element]
    type = StatisticsVectorPostprocessor
    vpp = nl_wb_element
    stats = 'min max sum average ratio'
  [../]

  [./nl_wb_edgecuts]
    type = WorkBalance
    execute_on = initial
    system = nl
    outputs  = none
    balances = 'num_partition_sides'
  []

  [./bl_edgecuts]
    type = StatisticsVectorPostprocessor
    vpp = nl_wb_edgecuts
    stats = 'sum'
  [../]
[]
