# inlet temperature
T_in = 523.0

mdot = 10

pout = 7e6

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1.5
  xmax = 1.5
  ymin = -1.5
  ymax = 1.5
  zmin = 0
  zmax = 10
  nx = 3
  ny = 3
  nz = 10
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [u]
  []
[]

[Postprocessors]
  [core_outlet_pressure]
    type = Receiver
    default = ${pout}
  []

  [core_inlet_mdot]
    type = Receiver
    default = ${mdot}
  []

  [core_inlet_temperature]
    type = Receiver
    default = ${T_in}
  []

  [core_inlet_pressure]
    type = FunctionValuePostprocessor
    function = compute_inlet_pressure_fn
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [core_outlet_mdot]
    type = ScalePostprocessor
    value = core_inlet_mdot
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [bypass_mdot]
    type = Receiver
  []

  [inlet_mdot]
    type = Receiver
  []

  [outlet_mdot]
    type = Receiver
  []

  [core_outlet_temperature]
    type = FunctionValuePostprocessor
    function = compute_outlet_temperature_fn
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [core_pressure_drop]
    type = DifferencePostprocessor
    value1 = core_inlet_pressure
    value2 = core_outlet_pressure
  []
[]

[Functions]
  [compute_outlet_temperature_fn]
    type = ParsedFunction
    symbol_values = 'core_inlet_mdot core_inlet_temperature  1000'
    symbol_names = 'mdot            Tin                     Q'
    expression = 'Tin + Q / mdot'
  []

  [compute_inlet_pressure_fn]
    type = ParsedFunction
    symbol_values = 'core_inlet_mdot core_outlet_pressure  5000'
    symbol_names = 'mdot            pout                     C'
    expression = 'pout + C * mdot'
  []
[]

[MultiApps]
  [thm]
    type = TransientMultiApp
    input_files = thm_non_overlapping.i
    sub_cycling = true
    max_procs_per_app = 1
    print_sub_cycles = false
  []
[]

[Transfers]
  #### thm Transfers ####
  ## transfers from thm
  [core_inlet_mdot]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = core_inlet_mdot
    to_postprocessor = core_inlet_mdot
    reduction_type = maximum
    from_multi_app = thm
  []

  [core_inlet_temperature]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = core_inlet_temperature
    from_postprocessor = core_inlet_temperature
    reduction_type = maximum
    from_multi_app = thm
  []

  [core_outlet_pressure]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = core_outlet_pressure
    from_postprocessor = core_outlet_pressure
    reduction_type = maximum
    from_multi_app = thm
  []

  [bypass_mdot]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = bypass_mdot
    from_postprocessor = bypass_mdot
    reduction_type = maximum
    from_multi_app = thm
  []

  [inlet_mdot]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = inlet_mdot
    from_postprocessor = inlet_mdot
    reduction_type = maximum
    from_multi_app = thm
  []

  [outlet_mdot]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = outlet_mdot
    from_postprocessor = outlet_mdot
    reduction_type = maximum
    from_multi_app = thm
  []

  ## transfers to thm
  [core_outlet_mdot]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = core_outlet_mdot
    to_postprocessor = core_outlet_mdot
    to_multi_app = thm
  []

  [core_outlet_temperature]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = core_outlet_temperature
    to_postprocessor = core_outlet_temperature
    to_multi_app = thm
  []

  [core_inlet_pressure]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = core_inlet_pressure
    to_postprocessor = core_inlet_pressure
    to_multi_app = thm
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
[]
