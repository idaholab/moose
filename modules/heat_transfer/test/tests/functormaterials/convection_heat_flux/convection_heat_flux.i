T_solid = 300
T_fluid = 500
htc = 100

# q_solid = htc * (T_fluid - T_solid) = 100 * (500 - 300) = 20000
# q_fluid = -q_solid = -20000

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FunctorMaterials]
  [q_solid_fmat]
    type = ADConvectionHeatFluxFunctorMaterial
    heat_flux_name = q_solid
    to_solid = true
    T_solid = ${T_solid}
    T_fluid = ${T_fluid}
    htc = ${htc}
  []
  [q_fluid_fmat]
    type = ConvectionHeatFluxFunctorMaterial
    heat_flux_name = q_fluid
    to_solid = false
    T_solid = ${T_solid}
    T_fluid = ${T_fluid}
    htc = ${htc}
  []
[]

[Postprocessors]
  [q_solid_pp]
    type = ADElementExtremeFunctorValue
    functor = q_solid
    execute_on = 'INITIAL'
  []
  [q_fluid_pp]
    type = ElementExtremeFunctorValue
    functor = q_fluid
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
