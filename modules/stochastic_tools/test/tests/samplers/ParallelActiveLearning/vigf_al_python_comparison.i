#* Active learning with the VIGF (Variance Improvement for Global Fit)
#* acquisition function, on the toy function
#*
#*     f(x) = sin(x) + 2*x ,   x in [0, 5]
#*
#* Adapted directly from your VIGF_parallelAL.i example (which itself
#* matches the documented GenericActiveLearningSampler pattern), swapping
#* the 2D Normal-distributed heat-conduction problem for a 1D Uniform
#* domain and a trivial toy-function sub-app (vigf_al_sub.i).
#*
#* "2 initial points, then 10 newly added points":
#*   Every documented example of GenericActiveLearningSampler /
#*   PMCMCBase / AISActiveLearning uses `initial_values` as exactly ONE
#*   seed point (its length always equals the number of distributions),
#*   so there's no confirmed mechanism to seed with 2 points directly.
#*   Instead, num_parallel_proposals is held constant at 2 for the whole
#*   run, and the run does 6 steps total:
#*     - step 1  -> the first batch of 2 points ("Initial Points": no GP
#*                  exists yet to rank candidates, so this batch is
#*                  effectively a space-filling/random initial design)
#*     - steps 2-6 -> 5 more batches of 2 = 10 points ("Newly Added
#*                  Points", chosen by maximizing VIGF against the GP
#*                  trained on all points seen so far)
#*
#* I could not run this file myself (no MOOSE build / no network in this
#* sandbox) and don't have source for GenericActiveLearningSampler,
#* GenericActiveLearner, ActiveLearningGaussianProcess, or
#* MultiAppSamplerControl, so treat this as a best-effort first draft --
#* please send me the exact error if something doesn't match your build.
#*
#* CONFIRMED from a real run: MultiAppSamplerControl passes each sampled
#* value to the sub-app as a plain CLI_ARGS override (e.g. "x_input=2.5"),
#* using param_names exactly as written -- it is NOT a control-tag lookup.
#* param_names must therefore be the sub-app's full parameter path, e.g.
#* 'Postprocessors/x_input/value', not just the block name 'x_input'.

[StochasticTools]
[]

[Distributions]
  [x_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 5
  []
[]

[ParallelAcquisition]
  [vigf]
    type = VarianceImprovementGlobalFit
  []
[]

[Samplers]
  [sample]
    type = GenericActiveLearningSampler
    distributions = 'x_dist'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 2
    num_tries = 1000
    seed = 100
    num_random_seeds = 100
    initial_values = '2.5'
    execute_on = 'PRE_MULTIAPP_SETUP timestep_end'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = vigf_al_sub.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'Postprocessors/x_input/value'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [conditional]
    type = GenericActiveLearner
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    acquisition = 'vigf'
    execute_on = 'timestep_begin'
  []
[]

[VectorPostprocessors]
  [sample_data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial timestep_end'
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 500
    learning_rate = 0.01
    # batch_size kept small since the total dataset here never exceeds 12
    # points (2 initial + 10 added) -- your example used 350 for a much
    # larger dataset.
    batch_size = 2
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcessSurrogate
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 4.0
    noise_variance = 1e-6
    length_factor = '4.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 6
[]

[Outputs]
  file_base = 'vigf_al_out'
  csv = true
  # JSON output is kept alongside CSV because the "constant" Reporter's
  # data is nested (constant/reporter_transfer:average:value growing over
  # rounds), which your example also output as JSON rather than CSV --
  # the nested structure may not flatten cleanly into the CSV table.
  [json]
    type = JSON
    execute_system_information_on = NONE
  []
[]
