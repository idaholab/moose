# Binary diffusion coefficient of N2 and O2 at 1e5 Pa and 1000 K. Cantera 3.2.0 gives
# 1.6508e-4 m^2/s with the data of gri30.yaml.

p = 1e5
T = 1000

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [n2]
    type = NASA7IdealGasFluidProperties
    species = N2
  []
  [o2]
    type = NASA7IdealGasFluidProperties
    species = O2
  []
  [mixture_fp]
    type = IdealGasMixtureFluidProperties
    component_fluid_properties = 'n2 o2'
  []
[]

[Materials]
  [pT_mat]
    type = ADGenericConstantMaterial
    prop_names = 'p T'
    prop_values = '${p} ${T}'
  []
  [test_mat]
    type = FluidPropertiesBinaryDiffusionCoefMaterial
    vapor_mixture_fp = mixture_fp
  []
[]

[Postprocessors]
  [diffcoef]
    type = ADElementAverageMaterialProperty
    mat_prop = mass_diffusion_coefficient
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
