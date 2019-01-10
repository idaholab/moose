[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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

[Postprocessors]
  [./elapsed]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
  [./res_calls]
    type = PerfGraphData
    section_name = "FEProblem::computeResidualInternal"
    data_type = calls
  [../]
  [./jac_calls]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = calls
  [../]
  [./jac_total_time]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = self
  [../]
  [./jac_average_time]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = total_avg
  [../]
  [./jac_total_time_with_sub]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = total
  [../]
  [./jac_average_time_with_sub]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = total_avg
  [../]
  [./jac_percent_of_active_time]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = self_percent
  [../]
  [./jac_percent_of_active_time_with_sub]
    type = PerfGraphData
    section_name = "FEProblem::computeJacobianInternal"
    data_type = total_percent
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph = true
[]
