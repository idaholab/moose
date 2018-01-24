[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    initial_condition = 0.01
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./nodal]
    type = AverageNodalVariableValue
    variable = u
    execute_on = 'TIMESTEP_END'
  [../]
  [./elemental]
    type = ElementAverageValue
    variable = u
    execute_on = 'TIMESTEP_END'
  [../]
  [./general]
    type = PointValue
    point = '0.5 0.5 0'
    variable = u
    execute_on = 'TIMESTEP_END'
  [../]
  [./internal_side]
    type = NumInternalSides
  [../]
  [./side]
    type = SideAverageValue
    boundary = right
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]

[Controls]
  [./pp_control]
    type = TimePeriod
    enable_objects = '*/nodal */elemental */general */internal_side */side'
    start_time = 0.5
    end_time = 1
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]
