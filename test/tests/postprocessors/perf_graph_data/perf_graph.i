[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Postprocessors]
  # Getting this information on INITIAL has no practical use, but
  # we want to make sure that we can obtain information about
  # a section that has not ran yet.
  [self]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CALLS
    must_exist = false
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [children]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CHILDREN
    execute_on = 'TIMESTEP_END'
  []
  [total]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = SELF
    execute_on = 'TIMESTEP_END'
  []
  [self_avg]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = SELF_AVG
    execute_on = 'TIMESTEP_END'
  []
  [children_avg]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CHILDREN_AVG
    execute_on = 'TIMESTEP_END'
  []
  [total_avg]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = TOTAL_AVG
    execute_on = 'TIMESTEP_END'
  []
  [self_percent]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = SELF_PERCENT
    execute_on = 'TIMESTEP_END'
  []
  [children_percent]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CHILDREN_PERCENT
    execute_on = 'TIMESTEP_END'
  []
  [total_percent]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = TOTAL_PERCENT
    execute_on = 'TIMESTEP_END'
  []
  [self_memory]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = SELF_MEMORY
    execute_on = 'TIMESTEP_END'
  []
  [children_memory]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CHILDREN_MEMORY
    execute_on = 'TIMESTEP_END'
  []
  [total_memory]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = TOTAL_MEMORY
    execute_on = 'TIMESTEP_END'
  []
  [calls]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CALLS
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
