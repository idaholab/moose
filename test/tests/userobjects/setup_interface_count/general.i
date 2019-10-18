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
    bottom_left = '0.5 0 0'
    block_id = 1
    top_right = '1 1 0'
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
    type = GeneralSetupInterfaceCount
    count_type = 'initial'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./timestep] # 10, once per timestep
    type = GeneralSetupInterfaceCount
    count_type = 'timestep'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./subdomain] # 0, method not implemented for GeneralUserObjects
    type = GeneralSetupInterfaceCount
    count_type = 'subdomain'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./initialize] # 1 for initial and 2 for each timestep
    type = GeneralSetupInterfaceCount
    count_type = 'initialize'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./finalize] # 1 for initial and 2 for each timestep
    type = GeneralSetupInterfaceCount
    count_type = 'finalize'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./execute] # 1 for initial and 2 for each timestep
    type = GeneralSetupInterfaceCount
    count_type = 'execute'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
  [./threadjoin] # 0, not implemented
    type = GeneralSetupInterfaceCount
    count_type = 'threadjoin'
    execute_on = 'initial timestep_begin timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
