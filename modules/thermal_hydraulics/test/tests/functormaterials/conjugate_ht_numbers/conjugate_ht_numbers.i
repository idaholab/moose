p_fluid = 1e5
T_fluid = 300
T_solid = 500

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp_air]
    type = IdealGasFluidProperties
  []
[]

[FunctorMaterials]
  [q_fmat]
    type = ADConjugateHTNumbersFunctorMaterial
    Pr_name = Pr
    Gr_name = Gr
    p_fluid = ${p_fluid}
    T_fluid = ${T_fluid}
    T_solid = ${T_solid}
    length = 0.01
    fluid_properties = fp_air
  []
[]

[Postprocessors]
  [Pr]
    type = ADElementExtremeFunctorValue
    functor = Pr
    execute_on = 'INITIAL'
  []
  [Gr]
    type = ADElementExtremeFunctorValue
    functor = Gr
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
