p_fluid = 101325
T_fluid = 293
T_solid = 973
k_fluid = 0.05
dia = 0.02

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
    length = ${dia}
    fluid_properties = fp_air
  []
  [CC_htc]
    type = ADChurchillChuHTCFunctorMaterial
    Pr = Pr
    Gr = Gr
    k_fluid = ${k_fluid}
    length = ${dia}
    htc_name = htc
  []
[]

[Postprocessors]
  [htc]
    type = ADElementExtremeFunctorValue
    functor = htc
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
