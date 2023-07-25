[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[Materials]
  [D]
    # we need to make sure not to supply derivatives to have a
    # wrong Jacobian to force more iterations to test the output on
    type = ParsedMaterial
    property_name = D
    expression = 'u^2+0.1'
    coupled_variables = u
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = D
  []
  [time]
    type = TimeDerivative
    variable = u
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

[VectorPostprocessors]
  [nodes]
    type = NodalValueSampler
    boundary = top
    sort_by = x
    variable = u
    execute_on = 'INITIAL NONLINEAR LINEAR TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  verbose = true
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'LINEAR'
  []
[]
