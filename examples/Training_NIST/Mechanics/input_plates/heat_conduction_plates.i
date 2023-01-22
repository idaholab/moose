[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'mesh_in.e'
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
    block = fuel
    variable = temperature
    value = 10000
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    block = 'fuel clad'
  []
[]

[BCs]
  [walls]
    type = DirichletBC
    variable = temperature
    boundary = 'clad_wall'
    value = 300
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
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


