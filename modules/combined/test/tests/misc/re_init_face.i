[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    use_automatic_differentiation = false
    strain = FINITE
    add_variables = true
  []
[]

[Variables]
  [disp_x]
    order = FIRST
  []
  [disp_y]
    order = FIRST
  []
[]

[Materials]
  [elastic_tensor_cover]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10
    poissons_ratio = 0.3
    use_displaced_mesh = true
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
  []
[]

[Executioner]
  type = Transient
  dt = 0.3
  num_steps = 3
[]

[Postprocessors]
  [side_average]
    type = SideAverageValue
    boundary = right
    variable = disp_x
  []
[]
