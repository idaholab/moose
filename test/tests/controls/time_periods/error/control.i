[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
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

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Dampers]
  [./const_damp]
    type = ConstantDamper
    damping = 0.9
  [../]
[]

[Controls]
  [./damping_control]
    type = TimePeriod
    disable_objects = 'const_damp'

    # Note: These numbers are quoted to get around an issue when
    # overriding numeric types with vectors of numeric types
    # on the CLI. They are still interpreted as numbers.
    start_time = '0.25'
    end_time = '0.55'
    execute_on = 'initial timestep_begin'
  [../]
[]
