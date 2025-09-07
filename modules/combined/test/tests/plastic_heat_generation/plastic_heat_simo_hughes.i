[Mesh]
  [block]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [temperature]
    initial_condition = 300
  []
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Kernels]
  [TensorMechanics]
    displacements = 'disp_x disp_y'
    strain = FINITE
  []
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [mass_x]
    type = MassMatrix
    variable = disp_x
    density = density
    matrix_tags = 'mass'
  []
  [mass_y]
    type = MassMatrix
    variable = disp_y
    density = density
    matrix_tags = 'mass'
  []
  [mass_T]
    type = MassMatrix
    variable = temperature
    density = thermal_density
    matrix_tags = 'mass'
  []
  [plastic_heating]
    type = MatBodyForce
    variable = temperature
    material_property = plastic_heat
  []
[]

[BCs]
  [bottom_x]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    type = ExplicitDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [top_y]
    type = ExplicitFunctionDirichletBC
    variable = disp_y
    boundary = top
    function = '100*t'
  []
  [temp_bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 300
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e9
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y'
  []
  [plasticity]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 10e6
    hardening_constant = 100e6
  []
  [plastic_heat_energy]
    type = ComputeSimoHughesJ2PlasticHeatEnergy
    stress = stress
  []
  [heat_conduction_material]
    type = HeatConductionMaterial
    specific_heat = 500
    thermal_conductivity = 50
  []
  [heat_cond_density]
    type = ParsedMaterial
    expression = 'density * specific_heat'
    material_property_names = 'density specific_heat'
    property_name = 'thermal_density'
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 7800
  []
[]

[Executioner]
  type = Transient

  dt = 0.0001
  end_time = 0.1

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    first_order_vars = 'temperature'
    second_order_vars = 'disp_x disp_y'
  []
  abort_on_solve_fail = true
[]

[Postprocessors]
  [plastic_heat_avg]
    type = ElementAverageValue
    variable = temperature
    execute_on = 'initial timestep_end'
  []
  [total_plastic_heat]
    type = ElementIntegralMaterialProperty
    mat_prop = plastic_heat
    execute_on = 'initial timestep_end'
  []
  [effective_plastic_strain]
    type = ElementAverageValue
    variable = effective_plastic_strain
    execute_on = 'initial timestep_end'
  []
[]

[AuxVariables]
  [effective_plastic_strain]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [effective_plastic_strain]
    type = MaterialRealAux
    variable = effective_plastic_strain
    property = effective_plastic_strain
    execute_on = timestep_end
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
