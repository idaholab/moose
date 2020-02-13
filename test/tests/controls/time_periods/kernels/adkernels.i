[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff0]
    type = ADMatDiffusionTest
    variable = u
    ad_mat_prop = 0.05
    regular_mat_prop = 0.05
  []
  [diff1]
    type = ADMatDiffusionTest
    variable = u
    ad_mat_prop = 0.5
    regular_mat_prop = 0.5
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

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Controls]
  [diff]
    type = TimePeriod
    enable_objects = 'Kernel::diff0'
    disable_objects = '*::diff1'
    start_time = '0'
    end_time = '0.49'
  []
[]
