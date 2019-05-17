[Mesh]
  file = mammoth_eigenvalue_out_bison0_cp/LATEST
  # file = bison_ss_out_cp/LATEST
[]

[Problem]
  restart_file_base = mammoth_eigenvalue_out_bison0_cp/LATEST
  # restart_file_base = bison_ss_out_cp/LATEST
  # force_restart = true
  # skip_additional_restart_data = true
[]

[AuxVariables]
  [./power_density]
    # family = L2_LAGRANGE
    # order = FIRST
    # initial_condition = 5e2
  [../]
[]

[Variables]
  [./temp]
    # initial_condition = 400
  [../]
[]

[Kernels]
  [./heat_conduction]
     type = Diffusion
     variable = temp
  [../]
  [./heat_ie]
    type = TimeDerivative
    variable = temp
  [../]
  [./heat_source_fuel]
    type = CoupledForce
    variable = temp
    v = power_density
  [../]
[]

[BCs]
  [bc]
    type = DirichletBC
    variable = temp
    boundary = '0 1 2 3'
    value = 450
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  start_time = 0
  end_time = 10
  dt = 1.0
  # steady_state_detection = true

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-7
[]

[Postprocessors]
  [./temp_fuel_avg]
    type = ElementAverageValue
    variable = temp
    block = '0'
    execute_on = 'initial timestep_end'
  [../]
  [./pwr_density]
    type = ElementIntegralVariablePostprocessor
    block = '0'
    variable = power_density
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  perf_graph = true
  exodus = true
  color = true
  csv = true
[]
