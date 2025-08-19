[Mesh]
  [block]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Variables]

  [T]
    initial_condition = '${units 25 degC}'
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [contract_x]
    type = FunctionAux
    variable = disp_x
    function = '-100*t*x'
    execute_on = 'TIMESTEP_BEGIN'
  []
  [contract_y]
    type = FunctionAux
    variable = disp_y
    function = '-100*t*y'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Kernels]

  [hc]
    type = HeatConduction
    variable = T
    use_displaced_mesh = true
    displacements = 'disp_x disp_y'
  []

  [mass_T]
    type = MassMatrix
    variable = T
    density = thermal_density
    matrix_tags = 'mass'
  []
  [hs]
    type = MatBodyForce
    variable = T
    use_displaced_mesh = false
    displacements = 'disp_x disp_y'
    material_property = heat_source
  []
[]

[Materials]
  [heat_source]
    type = ParsedMaterial
    expression = '-1e12 * y'
    property_name = 'heat_source'
    extra_symbols = 'y'
  []
  [heat_conduction_material]
    type = HeatConductionMaterial
    specific_heat = '${units 1 J/g/degC -> J/kg/degC}'
    thermal_conductivity = '${units 200 W/m/degC}'
  []
  [heat_cond_density]
    type = ParsedMaterial
    expression = 'density * specific_heat'
    material_property_names = 'density specific_heat'
    property_name = 'thermal_density'
    epsilon = 0.0
  []

  [slug_density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = '${units 100000 kg/m^3}'
  []
[]

[Executioner]
  type = Transient

  num_steps = 10

  [TimeStepper]
    type = ConstantDT
    dt = '${units 1e-4 s}'

  []
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    first_order_vars = 'T'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [crit_heat_timestep]
    type = CriticalTimeStepHeatConduction
    use_displaced_mesh = true
  []
[]

[Outputs]
  csv = true
  exodus = true
[]

