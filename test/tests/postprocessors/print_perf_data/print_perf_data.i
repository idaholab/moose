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
  [./res_calls]
    type = PerformanceData
    column = n_calls
    event = compute_residual()
  [../]
  [./jac_calls]
    type = PerformanceData
    column = n_calls
    event = compute_jacobian()
  [../]
  [./jac_total_time]
    type = PerformanceData
    column = total_time
    event = compute_jacobian()
  [../]
  [./jac_average_time]
    type = PerformanceData
    column = average_time
    event = compute_jacobian()
  [../]
  [./jac_total_time_with_sub]
    type = PerformanceData
    column = total_time_with_sub
    event = compute_jacobian()
  [../]
  [./jac_average_time_with_sub]
    type = PerformanceData
    column = average_time_with_sub
    event = compute_jacobian()
  [../]
  [./jac_percent_of_active_time]
    type = PerformanceData
    column = percent_of_active_time
    event = compute_jacobian()
  [../]
  [./jac_percent_of_active_time_with_sub]
    type = PerformanceData
    column = percent_of_active_time_with_sub
    event = compute_jacobian()
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  print_linear_residuals = true

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
  postprocessor_csv = true
[]
