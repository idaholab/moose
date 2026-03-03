[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 2
  ymax = 2
[]

[AuxVariables]
  [var1]
    initial_condition = 1
  []
  [var2]
    initial_condition = 2
  []
[]

[Postprocessors]
  [average1]
    type = ElementAverageValue
    variable = var1
    execute_on = 'initial timestep_end'
  []
  [average2]
    type = ElementAverageValue
    variable = var2
    execute_on = 'initial timestep_end'
  []
  [kokkos_average1]
    type = KokkosElementAverageValue
    variable = var1
    execute_on = 'initial timestep_end'
  []
  [kokkos_average2]
    type = KokkosElementAverageValue
    variable = var2
    execute_on = 'initial timestep_end'
  []
  [kokkos_sum]
    type = KokkosSumPostprocessor
    values = 'kokkos_average1 kokkos_average2'
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
