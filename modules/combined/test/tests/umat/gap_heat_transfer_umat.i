[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  temperature = temp
[]

[Mesh]
  file = gap_heat_transfer_mesh.e
[]

[Functions]
  [disp]
    type = PiecewiseLinear
    x = '0 2.0'
    y = '0 1.0'
  []
  [temp]
    type = PiecewiseLinear
    x = '0     1'
    y = '273 2000'
  []
  [pressure_function]
    type = PiecewiseLinear
    x = '0     1'
    y = '0 200'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [temp]
    initial_condition = 273
  []
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = 2
    secondary = 3
    emissivity_primary = 0
    emissivity_secondary = 0
  []
[]

[Modules/TensorMechanics/Master/All]
  volumetric_locking_correction = true
  strain = FINITE
  generate_output = 'strain_yy stress_yy'
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
  []
[]

[BCs]
  [move_right]
    type = FunctionDirichletBC
    boundary = '3'
    variable = disp_x
    function = disp
  []

  [fixed_x]
    type = DirichletBC
    boundary = '1'
    variable = disp_x
    value = 0
  []
  [fixed_y]
    type = DirichletBC
    boundary = '1 2 4'
    variable = disp_y
    value = 0
  []
  [fixed_z]
    type = DirichletBC
    boundary = '1 2 3 4'
    variable = disp_z
    value = 0
  []

  [temp_bottom]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  []
  [temp_top]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  []
  [Pressure]
    [example]
      boundary = 3
      function = pressure_function
    []
  []
[]

[Materials]
  # 1. Active for umat calculation
  [umat]
    type = AbaqusUMATStress
    constant_properties = '1.0e6 0.3'
    plugin = '../../../../tensor_mechanics/test/plugins/elastic_temperature'
    num_state_vars = 0
    temperature = temp
    use_one_based_indexing = true
  []
  #  2. Active for reference MOOSE computations
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    base_name = 'base'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [temp_dependent_elasticity_tensor]
    type = CompositeElasticityTensor
    block = '1 2'
    args = temp
    tensors = 'base'
    weights = 'prefactor_material'
  []
  [prefactor_material_block]
    type = DerivativeParsedMaterial
    block = '1 2'
    property_name = prefactor_material
    coupled_variables = temp
    expression = '273/(temp)'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
  [heat]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 1.0
  []
  [density]
    type = Density
    block = '1 2'
    density = 1.0
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  dt = 0.1
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
