# Snap-through path of cubic_snapthrough.i stepped along instead of traced in
# one solve: every time step advances the trace by a single continuation
# increment of radius step_size * dt and commits the state it reaches, so the
# accumulated load factor rises to the peak of the path, falls through the
# snap, and recovers, parting from the time as soon as the path turns. Time is
# a pseudo parameter that counts arc steps; with the constant dt of 1 the
# radius is step_size itself.

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
    type = ADMatBodyForce
    variable = u
    material_property = restoring_force
    value = -1
  []
  [load]
    type = BodyForce
    variable = u
    value = 1
    vector_tags = 'arc_length_load'
  []
[]

[Materials]
  [cubic]
    type = ADParsedMaterial
    property_name = restoring_force
    coupled_variables = 'u'
    expression = 'u * (u - 1) * (u - 2)'
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.25
[]

[Postprocessors]
  [u_mid]
    type = PointValue
    variable = u
    point = '0.5 0 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [lambda]
    type = ArcLengthLoadParameter
  []
  [lambda_minus_time]
    type = ParsedPostprocessor
    expression = 'lambda - t'
    pp_names = 'lambda'
    use_t = true
  []
  [equilibrium_residual]
    type = ParsedPostprocessor
    expression = 'lambda - u_mid * (u_mid - 1) * (u_mid - 2)'
    pp_names = 'lambda u_mid'
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'u_mid'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  num_steps = 25
[]

[Outputs]
  csv = true
[]
