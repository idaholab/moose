[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./dt]
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

[Adaptivity]
  [./Markers]
    [./uni]
      type = UniformMarker
      mark = REFINE
    [../]
  [../]
  marker = uni
  stop_time = 7.5
[]

[Postprocessors]
  [./physical]
    type = MemoryUsage
    mem_type = physical_memory
    value_type = total
    execute_on = 'initial TIMESTEP_END nonlinear linear'
  [../]
  [./virtual]
    type = MemoryUsage
    mem_type = virtual_memory
    value_type = total
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./page_faults]
    type = MemoryUsage
    mem_type = page_faults
    value_type = total
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./DOFs]
    type = NumDOFs
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./walltime]
    type = PerformanceData
    event = ALIVE
    execute_on = 'initial TIMESTEP_END'
  [../]
[]

[Executioner]
  type = Transient

  #solve_type = 'PJFNK'
  #petsc_options_iname = '-pc_type -pc_hypre_type'
  #petsc_options_value = 'hypre boomeramg'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-10

  num_steps = 8
  dt = 1
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL TIMESTEP_END FINAL'
  print_perf_log = true
[]
