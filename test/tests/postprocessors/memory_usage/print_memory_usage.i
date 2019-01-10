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

  # this marker will tag every element for refinement, growing the problem
  # exponentially with each timestep
  marker = uni

  # avoid a refine after the final step
  stop_time = 4.5
[]

[Postprocessors]
  [./physical]
    type = MemoryUsage
    mem_type = physical_memory
    value_type = total
    # by default MemoryUsage reports the peak value for the current timestep
    # out of all samples that have been taken (at linear and non-linear iterations)
    execute_on = 'INITIAL TIMESTEP_END NONLINEAR LINEAR'
  [../]
  [./virtual]
    type = MemoryUsage
    mem_type = virtual_memory
    value_type = total
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./page_faults]
    type = MemoryUsage
    mem_type = page_faults
    value_type = total
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./DOFs]
    type = NumDOFs
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./walltime]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      lu'

  nl_abs_tol = 1e-10

  num_steps = 5
  dt = 1
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL TIMESTEP_END FINAL'
  perf_graph = true
[]
