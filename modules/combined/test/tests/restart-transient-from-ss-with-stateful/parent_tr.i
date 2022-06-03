[Problem]
  restart_file_base = parent_ss_checkpoint_cp/LATEST
  force_restart = true
  skip_additional_restart_data = true
[]

[Mesh]
  file = parent_ss_checkpoint_cp/LATEST
[]

[Variables]
  [./temp]
    # no initial condition for restart.
  [../]
[]

[AuxVariables]
  [./power]
    order = FIRST
    family = L2_LAGRANGE
    initial_condition = 350
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]
  [./heat_source_fuel]
    type = CoupledForce
    variable = temp
    v = 'power'
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = temp
    boundary = 'bottom top left right'
    value = 300
  [../]
[]

[Materials]
  [./heat_material]
    type = HeatConductionMaterial
    temp = temp
    specific_heat = 1000
    thermal_conductivity = 500
  [../]
  [./density]
    type = Density
    density = 2000
  [../]
[]

[Postprocessors]
  [./avg_temp]
    type = ElementAverageValue
    variable = temp
    execute_on = 'timestep_end'
  [../]
  [./avg_power]
    type = ElementAverageValue
    variable = power
    execute_on = 'timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 300'
  line_search = 'none'

  l_tol = 1e-02
  nl_rel_tol = 5e-05
  nl_abs_tol = 5e-05

  l_max_its = 50
  nl_max_its = 25

  start_time = 0
  end_time = 40
  dt = 10
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  color = true
  exodus = true
[]

[MultiApps]
  [./bison]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = 'sub_tr.i'
    execute_on = 'timestep_end'
  [../]
[]

[Transfers]
  [./to_bison_mechanics]
    type = MultiAppProjectionTransfer
    to_multi_app = bison
    variable = temp
    source_variable = temp
    execute_on = 'timestep_end'
  [../]
[]
