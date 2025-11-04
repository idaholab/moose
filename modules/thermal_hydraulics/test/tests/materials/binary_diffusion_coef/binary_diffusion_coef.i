p = 1e5
T = 300
collision_diam = 0.3e-9

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp1]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.029
  []
  [fp2]
    type = IdealGasFluidProperties
    gamma = 1.5
    molar_mass = 0.04
  []
  [mixture_fp]
    type = IdealGasMixtureFluidProperties
    component_fluid_properties = 'fp1 fp2'
  []
[]

[Materials]
  [pT_mat]
    type = ADGenericConstantMaterial
    prop_names = 'p T'
    prop_values = '${p} ${T}'
  []
  [test_mat]
    type = BinaryDiffusionCoefMaterial
    primary_collision_diameter = ${collision_diam}
    secondary_collision_diameter = ${collision_diam}
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
