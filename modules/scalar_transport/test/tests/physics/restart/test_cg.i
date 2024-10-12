[Mesh]
  active = 'cmg'
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = 10
    dy = 10
  []
  [fmg_restart]
    type = FileMeshGenerator
    file = user_ics.e
    use_for_exodus_restart = true
  []
[]

[Debug]
  show_actions=true
[]

[Physics]
  [MultiSpeciesDiffusion]
    [ContinuousGalerkin]
      [diff]
        # A and C have the same equation, on purpose
        species = 'A B C'

        diffusivity_matprops = '1 1 1'

        source_functors = '0 2 0'
        source_coefs = '1 2 1'
      []
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  verbose = true
[]

[Problem]
  solve = false
[]

[Outputs]
  # Used to set up a restart from checkpoint
  checkpoint = true
  # Used to set up a restart from exodus file
  [exodus]
    type = Exodus
    execute_on = TIMESTEP_END
  []
  # Used to check results
  csv = true
  execute_on = INITIAL
[]

[Postprocessors]
  [min_A]
    type = ElementExtremeValue
    variable = 'A'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_A]
    type = ElementExtremeValue
    variable = 'A'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_B]
    type = ElementExtremeValue
    variable = 'B'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_B]
    type = ElementExtremeValue
    variable = 'B'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_C]
    type = ElementExtremeValue
    variable = 'C'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_C]
    type = ElementExtremeValue
    variable = 'C'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
[]
