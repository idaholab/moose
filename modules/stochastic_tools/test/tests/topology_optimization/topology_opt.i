num_samples = 50
reset_freq = 15
T_init = 100
T_final = 0.1
beta = ${fparse pow(T_final / T_init, 1.0 / num_samples)}

[StochasticTools]
[]

[UserObjects]
  [yes_constraint]
    type = YesConstraint
  []
[]

[Samplers]
  [sample]
    type = TopologyOptimizer
    num_parallel_proposals = 1
    configuration_size = 16
    seed = 4
    constraints = 'yes_constraint'
    num_iterations = ${num_samples}
  []
[]

[Reporters]
  [TopOptReporter]
    type = TopologyOptimizerDecision
    temperature_schedule = temperature
    sampler = sample
    print_decisions = true
    reset_frequency = ${reset_freq}
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Functions]
  [temperature]
    type = ParsedFunction
    expression = 'pow(${beta}, t) * ${T_init}'
  []
[]

[Postprocessors]
  [temperature]
     type = FunctionValuePostprocessor
     function = temperature
  []
[]

[Transfers]
  [initial_mesh]
    type = InitialTopologicalMeshTransfer
    from_multi_app = sub
  []

  [topological_opt_1]
    type = TopologicalOptimizationTransfer
    decision_reporter = TopOptReporter
    to_multi_app = sub
  []

  [topological_opt_2]
    type = TopologicalOptimizationTransfer
    decision_reporter = TopOptReporter
    objective_name = cost
    from_multi_app = sub
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
[]
