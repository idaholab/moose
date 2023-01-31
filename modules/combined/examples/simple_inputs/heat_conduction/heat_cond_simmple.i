[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    value = 10000
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    block = 0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = 'left right'
    value = 200
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [peak_temperature]
    type = NodalExtremeValue
    variable = temperature
  []
[]
