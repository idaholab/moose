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
  [calls]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CALLS
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [self]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = SELF
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [children]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = CHILDREN
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total]
    type = PerfGraphData
    section_name = FEProblem::computeResidualInternal
    data_type = TOTAL
    execute_on = 'INITIAL TIMESTEP_END'
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
  [pgraph]
    type = PerfGraphOutput
    level = 1
    heaviest_branch = true
    heaviest_sections = 10
  []
[]
