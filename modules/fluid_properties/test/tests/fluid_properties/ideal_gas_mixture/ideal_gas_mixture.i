[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp1]
    type = IdealGasFluidProperties
  []
  [fp2]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 1000
    p_inf = 0
    q = 0
  []
  [fpmix]
    type = IdealGasMixtureFluidProperties
    component_fluid_properties = 'fp1 fp2'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
