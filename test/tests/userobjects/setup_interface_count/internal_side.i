[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [./right_side]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 0.5 0'
    block_id = 1
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./initial] # 1 per simulation
    type = InternalSideSetupInterfaceCount
    count_type = 'initial'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./timestep] # once per timestep
    type = InternalSideSetupInterfaceCount
    count_type = 'timestep'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./subdomain] # 1 on initial and 2 for each timestep
    type = InternalSideSetupInterfaceCount
    count_type = 'subdomain'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./initialize] # 1 for initial and 2 for each timestep
    type = InternalSideSetupInterfaceCount
    count_type = 'initialize'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./finalize] # 1 for initial and 2 for each timestep
    type = InternalSideSetupInterfaceCount
    count_type = 'finalize'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./execute] # 4 for initial and 8 for each timestep
    type = InternalSideSetupInterfaceCount
    count_type = 'execute'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./threadjoin] # 1 for initial and 2 for each timestep
    type = InternalSideSetupInterfaceCount
    count_type = 'threadjoin'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
