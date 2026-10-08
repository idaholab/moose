[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 0.5
    xmin = -0.5
  []
  [left_line]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '-0.5 0 0'
    top_right = '0 0 0'
    block_id = 1
    block_name = 'left_line'
    location = INSIDE
  []
  [right_line]
    type = SubdomainBoundingBoxGenerator
    input = left_line
    bottom_left = '0 0 0'
    top_right = '0.5 0 0'
    block_id = 2
    block_name = 'right_line'
    location = INSIDE
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [time_derivative]
    # type = HeatConductionTimeDerivative
    type = KokkosTrussHeatConductionTimeDerivative
    variable = temperature
    area = area
  []
  [heat_conduction]
    # type = KokkosHeatConduction
    type = KokkosTrussHeatConduction
    variable = temperature
    area = area
  []
[]

[AuxVariables]
  [area]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [area]
    type = KokkosConstantAux
    variable = area
    value = 0.1
    execute_on = 'initial timestep_begin'
  []
[]

[Materials]
  [left_line]
    type = KokkosGenericConstantMaterial
    block = 'left_line'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '0.1                 1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
  [right_line]
    type = KokkosGenericConstantMaterial
    block = 'right_line'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '5.0e-3                  1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
[]

[BCs]
  [right]
    # the reference applies 10 * t, which equals 10 in the only time step ending at t = 1
    type = KokkosDirichletBC
    variable = temperature
    boundary = 'right'
    value = 10
  []
[]


[Executioner]
  type = Transient
  start_time = 0
  dt = 1
  end_time = 1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
