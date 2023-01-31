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
    type = CoupledForce
    block = fuel
    variable = temperature
    v = power_density
  []
[]

[Materials]
  [heat_conductor_fuel]
    type = HeatConductionMaterial
    thermal_conductivity = 0.1
    block = 'fuel'
  []
  [heat_conductor_clad]
    type = HeatConductionMaterial
    thermal_conductivity = 167
    block = 'clad'
  []
  [heat_conductor_water]
    type = HeatConductionMaterial
    thermal_conductivity = 0.6
    block = 'water'
  []
[]

[BCs]
  [walls]
    type = DirichletBC
    variable = temperature
    boundary = 'clad_wall bottom_to_clad top_to_clad'
    value = 300
  []
[]

[AuxVariables]
  [power_density]
    family = L2_LAGRANGE
    order = FIRST
    block = fuel
    initial_condition = 1e9
  []
  [Tfuel]
    block = fuel
  []
  [Tclad]
    block = clad
  []
[]

[AuxKernels]
  [Tfuel_aux]
    type = CopyAux
    variable = Tfuel
    copy_variable = temperature
  []
  [Tclad_aux]
    type = CopyAux
    variable = Tclad
    copy_variable = temperature
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
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
