[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  parallel_type = replicated
  uniform_refine = 1
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
  [time]
    type = TimeDerivative
    variable = u
  []
  [sink]
    type = BodyForce
    variable = u
    value = -1
  []
[]

[BCs]
  [right]
    type = PostprocessorDirichletBC
    variable = u
    boundary = right
    postprocessor = 'from_main'
  []
[]

[Postprocessors]
  [from_main]
    type = Receiver
    default = 0
  []
  [to_main]
    type = SideAverageValue
    variable = u
    boundary = left
  []
  [average]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-14

  fixed_point_algorithm = 'steffensen'
[]

[Outputs]
  [csv]
    type = CSV
    start_step = 6
  []
  exodus = false
[]
