[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 8
    ny = 8
    xmin = -82.627
    xmax = 82.627
    ymin = -82.627
    ymax = 82.627
    dim = 2
  []
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 500
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
    execute_on = 'initial timestep_end'
  [../]
  [./avg_power]
    type = ElementAverageValue
    variable = power
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 300'
  line_search = 'none'

  l_tol = 1e-05
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-9

  l_max_its = 50
  nl_max_its = 25
[]

[Outputs]
  perf_graph = true
  color = true
  exodus = true

  [checkpoint]
    type = Checkpoint
    num_files = 2
    additional_execute_on = 'FINAL' # seems to be a necessary to avoid a Checkpoint bug
  []
[]

[MultiApps]
  [./bison]
    type = FullSolveMultiApp
    positions = '0 0 0'
    input_files = 'sub_ss.i'
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
