# Q2PPiecewiseLinearSink (and the Flux Postprocessor)
# There are three sinks: water with no relperm and density;
# water with relperm and density; gas with relperm and density.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]



[UserObjects]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.5
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./RelPermGas]
    type = Q2PRelPermPowerGas
    simm = 0.0
    n = 3
  [../]
[]

[Variables]
  [./pp]
    [./InitialCondition]
      type = FunctionIC
      function = 1
    [../]
  [../]
  [./sat]
    [./InitialCondition]
      type = FunctionIC
      function = 0.5
    [../]
  [../]
[]

[Q2P]
  porepressure = pp
  saturation = sat
  water_density = DensityWater
  water_relperm = RelPermWater
  water_viscosity = 0.8
  gas_density = DensityGas
  gas_relperm = RelPermGas
  gas_viscosity = 0.5
  diffusivity = 0.0
  output_total_masses_to = 'CSV'
  save_gas_flux_in_Q2PGasFluxResidual = true
  save_water_flux_in_Q2PWaterFluxResidual = true
  save_gas_Jacobian_in_Q2PGasJacobian = true
  save_water_Jacobian_in_Q2PWaterJacobian = true
[]

[Postprocessors]
  [./left_water_out]
    type = Q2PPiecewiseLinearSinkFlux
    boundary = left
    porepressure = pp
    pressures = '0 1'
    bare_fluxes = '0 1.5'
    multiplying_fcn = 0.1
    execute_on = 'initial timestep_end'
  [../]
  [./right_water_out]
    type = Q2PPiecewiseLinearSinkFlux
    boundary = right
    porepressure = pp
    pressures = '0 1'
    bare_fluxes = '1 2'
    fluid_density = DensityWater
    fluid_viscosity = 0.8
    fluid_relperm = RelPermWater
    saturation = sat
    execute_on = 'initial timestep_end'
  [../]
  [./right_gas_out]
    type = Q2PPiecewiseLinearSinkFlux
    boundary = right
    porepressure = pp
    pressures = '0 1'
    bare_fluxes = '1 1'
    fluid_density = DensityGas
    fluid_viscosity = 0.5
    fluid_relperm = RelPermGas
    saturation = sat
    execute_on = 'initial timestep_end'
  [../]
  [./p_left]
    type = PointValue
    point = '0 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
  [./sat_left]
    type = PointValue
    point = '0 0 0'
    variable = sat
    execute_on = 'initial timestep_end'
  [../]
  [./p_right]
    type = PointValue
    point = '1 0 0'
    variable = pp
    execute_on = 'initial timestep_end'
  [../]
  [./sat_right]
    type = PointValue
    point = '1 0 0'
    variable = sat
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
  [./left_water]
    type = Q2PPiecewiseLinearSink
    boundary = left
    pressures = '0 1'
    bare_fluxes = '0 1.5'
    multiplying_fcn = 0.1
    variable = sat
    other_var = pp
    var_is_porepressure = false
    use_mobility = false
    use_relperm = false
    fluid_density = DensityWater
    fluid_viscosity = 0.8
    fluid_relperm = RelPermWater
  [../]
  [./right_water]
    type = Q2PPiecewiseLinearSink
    boundary = right
    pressures = '0 1'
    bare_fluxes = '1 2'
    variable = sat
    other_var = pp
    var_is_porepressure = false
    use_mobility = true
    use_relperm = true
    fluid_density = DensityWater
    fluid_viscosity = 0.8
    fluid_relperm = RelPermWater
  [../]
  [./right_gas]
    type = Q2PPiecewiseLinearSink
    boundary = right
    pressures = '0 1'
    bare_fluxes = '1 1'
    variable = pp
    other_var = sat
    var_is_porepressure = true
    use_mobility = true
    use_relperm = true
    fluid_density = DensityGas
    fluid_viscosity = 0.5
    fluid_relperm = RelPermGas
  [../]
[]

[AuxVariables]
  [./one]
    initial_condition = 1
  [../]
[]

[Materials]
  [./rock]
    type = Q2PMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-2 0 0  0 1E-2 0  0 0 1E-2'
    gravity = '0 0 0'
  [../]
[]


[Preconditioning]
  active = 'andy'
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.1
  end_time = 0.5
[]

[Outputs]
  file_base = q2p01
  [./CSV]
    type = CSV
  [../]
[]
