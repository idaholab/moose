# Part 2: read the mesh from the checkpoint written by part1 and seed the initial conditions of
# the nodal, high-order elemental and scalar variables from that checkpoint's stored solution
# using 'initial_from_file_var'. No time step is taken; the restored initial state is what is
# checked (it must match part1's final state). 'initial_from_file_timestep' defaults to LATEST,
# which is required for checkpoint files (the checkpoint file itself selects the state).

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = part1_out_cp/LATEST
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    initial_from_file_var = u
  []
  [ue]
    family = MONOMIAL
    order = FIRST
    initial_from_file_var = ue
  []
  [lambda]
    family = SCALAR
    order = FIRST
    initial_from_file_var = lambda
  []
[]

[Kernels]
  [u_time]
    type = TimeDerivative
    variable = u
  []
  [u_diff]
    type = Diffusion
    variable = u
  []
  [ue_time]
    type = TimeDerivative
    variable = ue
  []
  [ue_reaction]
    type = Reaction
    variable = ue
  []
  [ue_source]
    type = BodyForce
    variable = ue
    function = 'x + 2*y + 3*t'
  []
[]

[ScalarKernels]
  [lambda_time]
    type = ODETimeDerivative
    variable = lambda
  []
  [lambda_source]
    type = ParsedODEKernel
    variable = lambda
    expression = '-1'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [u_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'INITIAL'
  []
  [ue_integral]
    type = ElementIntegralVariablePostprocessor
    variable = ue
    execute_on = 'INITIAL'
  []
  [lambda_value]
    type = ScalarVariable
    variable = lambda
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 0
  dt = 0.5
[]

[Outputs]
  exodus = true
  csv = true
[]
