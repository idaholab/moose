rho = 3.1176
vel = 100
k = 0.38220
mu = 4.8587e-05
cp = 5189.8
p = 100e3
T = 1073
T_wall = 1074
D_inner = 0.01
D_outer = 0.015
length = 0.5

[GlobalParams]
  execute_on = 'INITIAL'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'rho vel k mu cp p T T_wall'
    prop_values = '${rho} ${vel} ${k} ${mu} ${cp} ${p} ${T} ${T_wall}'
  []
  [test_material]
    type = ADWallHTCGnielinskiAnnularMaterial
    htc_wall = htc_wall
    D_inner = ${D_inner}
    D_outer = ${D_outer}
    channel_length = ${length}
    at_inner_wall = true
    fluid_is_gas = true
    gas_heating_correction_exponent = 0.15
    fluid_properties = fp
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [htc_wall]
    type = ADElementAverageMaterialProperty
    mat_prop = htc_wall
  []
[]

[Outputs]
  csv = true
[]

