[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
[]

[Variables]
  [w]
  []
[]

[AuxVariables]
  [v]
  []
[]

[Kernels]
  [time_derivative]
    type = TimeDerivative
    variable = w
  []
  [diffusion]
    type = Diffusion
    variable = w
  []
  [source]
    type = CoupledForce
    variable = w
    v = v
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = w
    boundary = '0'
    value = 0
  []
[]

[Postprocessors]
  [avg_v]
    type = ElementAverageValue
    variable = v
    execute_on = 'initial linear'
  []
  [avg_w]
    type = ElementAverageValue
    variable = w
    execute_on = 'initial linear'
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  end_time = 0.1
  dt = 0.02
  # steady_state_detection = true
[]


[Outputs]
  exodus = true
  # print_linear_residuals = false
[]
