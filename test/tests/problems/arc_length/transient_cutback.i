[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
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
  [restoring]
    type = Reaction
    variable = u
  []
  [load]
    type = BodyForce
    variable = u
    value = 10
    vector_tags = 'arc_length_load'
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.5
  max_continuation_steps = 17
[]

[Postprocessors]
  [lambda]
    type = ArcLengthLoadParameter
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 0.5
  [TimeSteppers]
    [adaptive]
      type = IterationAdaptiveDT
      dt = 0.5
    []
  []
[]
