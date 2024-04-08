T_solid = 500
T_fluid = 300
htc = 100

# q = htc * (T_solid - T_fluid) = 100 * (500 - 300) = 20000

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FunctorMaterials]
  [q_fmat]
    type = ADConvectionHeatFluxFunctorMaterial
    heat_flux_name = q
    T_solid = ${T_solid}
    T_fluid = ${T_fluid}
    htc = ${htc}
  []
[]

[Postprocessors]
  [q_pp]
    type = ADElementExtremeFunctorValue
    functor = q
    execute_on = 'INITIAL'
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
  execute_on = 'INITIAL'
[]
