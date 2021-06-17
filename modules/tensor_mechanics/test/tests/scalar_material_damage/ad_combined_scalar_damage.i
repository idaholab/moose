[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX8
[]

[AuxVariables]
  [damage_index]
    order = CONSTANT
    family = MONOMIAL
  []
  [damage_index_a]
    order = CONSTANT
    family = MONOMIAL
  []
  [damage_index_b]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    generate_output = 'stress_xx strain_xx'
    use_automatic_differentiation = true
  []
[]

[AuxKernels]
  [damage_index]
    type = ADMaterialRealAux
    variable = damage_index
    property = damage_index
    execute_on = timestep_end
  []
  [damage_index_a]
    type = ADMaterialRealAux
    variable = damage_index_a
    property = damage_index_a
    execute_on = timestep_end
  []
  [damage_index_b]
    type = ADMaterialRealAux
    variable = damage_index_b
    property = damage_index_b
    execute_on = timestep_end
  []
[]

[BCs]
  [symmy]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [axial_load]
    type = ADDirichletBC
    variable = disp_x
    boundary = right
    value = 0.01
  []
[]

[Functions]
  [damage_evolution_a]
    type = PiecewiseLinear
    xy_data = '0.0   0.0
               0.1   0.0
               2.1   2.0'
  []
  [damage_evolution_b]
    type = PiecewiseLinear
    xy_data = '0.0   0.2
               0.1   0.2
               2.1   0.7'
  []
[]

[Materials]
  [damage_index_a]
    type = ADGenericFunctionMaterial
    prop_names = damage_index_prop_a
    prop_values = damage_evolution_a
  []
  [damage_index_b]
    type = ADGenericFunctionMaterial
    prop_names = damage_index_prop_b
    prop_values = damage_evolution_b
  []
  [damage_a]
    type = ADScalarMaterialDamage
    damage_index = damage_index_prop_a
    damage_index_name = damage_index_a
  []
  [damage_b]
    type = ADScalarMaterialDamage
    damage_index = damage_index_prop_b
    damage_index_name = damage_index_b
  []
  [damage]
    type = ADCombinedScalarDamage
    damage_models = 'damage_a damage_b'
  []
  [stress]
    type = ADComputeDamageStress
    damage_model = damage
  []
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.2
    youngs_modulus = 10e9
  []
[]

[Postprocessors]
  [stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  []
  [strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []
  [damage_index]
    type = ElementAverageValue
    variable = damage_index
  []
  [damage_index_a]
    type = ElementAverageValue
    variable = damage_index_a
  []
  [damage_index_b]
    type = ElementAverageValue
    variable = damage_index_b
  []
[]

[Executioner]
  type = Transient

  l_max_its  = 50
  l_tol      = 1e-8
  nl_max_its = 20
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8

  dt = 0.1
  dtmin = 0.1
  end_time = 1.1
[]

[Outputs]
  csv=true
[]
