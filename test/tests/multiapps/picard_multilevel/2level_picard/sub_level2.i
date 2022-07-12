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
  [dirichlet0]
    type = DirichletBC
    variable = w
    boundary = '3'
    value = 0
  []
  [dirichlet]
    type = DirichletBC
    variable = w
    boundary = '1'
    value = 100
  []
[]

[Postprocessors]
  [avg_v]
    type = ElementAverageValue
    variable = v
    execute_on = 'initial timestep_begin timestep_end'
  []
  [avg_w]
    type = ElementAverageValue
    variable = w
    execute_on = 'initial  timestep_begin timestep_end'
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  end_time = 0.1
  dt = 0.02
[]


[Outputs]
  exodus = true
  [screen]
    type = Console
    execute_postprocessors_on= "timestep_end timestep_begin"
  []
[]
