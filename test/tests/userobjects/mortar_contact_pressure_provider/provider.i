# The secondary face y = 0.5, 0 <= x <= 1, stores the scaled pressure scaling * (1 + x). The
# integral of the recovered physical pressure 1 + x over the face is 1.5, which linear interpolation
# of the nodal pressures reproduces exactly.
scaling = 1e3

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../mortar/continuity-2d-conforming/2blk-conf.e
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '101'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '100'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [scaled_pressure]
    block = 'secondary_lower'
    [InitialCondition]
      type = FunctionIC
      function = '${scaling} * (1 + x)'
    []
  []
[]

[UserObjects]
  [provider]
    type = TestMortarContactPressureProvider
    scaled_pressure = scaled_pressure
    scaling = ${scaling}
    primary_boundary = 100
    secondary_boundary = 101
    primary_subdomain = 10000
    secondary_subdomain = 10001
  []
  [integral]
    type = MortarContactPressureIntegral
    contact_pressure_uo = provider
    primary_boundary = 100
    secondary_boundary = 101
    primary_subdomain = 10000
    secondary_subdomain = 10001
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [contact_pressure_integral]
    type = MortarContactPressureIntegralPostprocessor
    integral = integral
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
