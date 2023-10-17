#
# This test replicates the legacy heat transfter test
# gap_heat_transfer_radiation/gap_heat_transfer_radiation_test.i
# The flux post processors give 3.753945e+01
#

[Mesh]
  [file]
    type = FileMeshGenerator
    file = gap_heat_transfer_radiation_test.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '200'
    new_block_name = 'secondary_lower'
    input = file
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '3'
    new_block_id = '300'
    new_block_name = 'primary_lower'
    input = secondary
  []
[]

[Functions]
  [temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '200 200'
  []
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
    scaling = 1e-8
  []
  [lm]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
    scaling = 1e-1
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
    block = '1 2'
  []
[]

[BCs]
  [temp_far_left]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  []

  [temp_far_right]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  []
[]

[UserObjects]
  [radiative]
    type = GapFluxModelRadiative
    secondary_emissivity = 0.5
    primary_emissivity = 0.5
    temperature = temp
    boundary = 3
  []
  [simple]
    type = GapFluxModelSimple
    k = 0.09187557
    temperature = temp
    boundary = 3
  []
[]

[Constraints]
  [ced]
    type = ModularGapConductanceConstraint
    variable = lm
    secondary_variable = temp
    primary_boundary = 3
    primary_subdomain = 300
    secondary_boundary = 2
    secondary_subdomain = 200
    gap_flux_models = 'simple radiative'
  []
[]

[Materials]
  [heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 10000000.0
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '1.0'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  end_time = 1.0
[]

[Postprocessors]
  [temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
    execute_on = 'initial timestep_end'
  []

  [temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
    execute_on = 'initial timestep_end'
  []

  [flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  []

  [flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  []
[]

[Outputs]
  exodus = true
[]
