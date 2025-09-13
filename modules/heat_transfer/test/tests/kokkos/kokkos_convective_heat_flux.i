[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [temp]
    initial_condition = 100.0
  []
[]

[KokkosKernels]
  [heat_conduction]
    type = KokkosHeatConduction
    variable = temp
  []
[]

[KokkosBCs]
  [left]
    type = KokkosDirichletBC
    variable = temp
    boundary = left
    value = 100.0
  []
  [right]
    type = KokkosConvectiveHeatFluxBC
    variable = temp
    boundary = right
    T_infinity = 200.0
    heat_transfer_coefficient = 10
  []
[]

[KokkosMaterials]
  [conduction]
    type = KokkosHeatConductionMaterial
    thermal_conductivity = 10
  []
  [bc]
    type = KokkosGenericConstantMaterial
    prop_names = 'Tinf htc'
    prop_values = '200 10'
  []
[]

[Postprocessors]
  [right_flux]
    type = SideDiffusiveFluxAverage
    variable = temp
    boundary = right
    diffusivity = 10
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
