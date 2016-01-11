# quick two phase with Pgas and Swater being variables

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 1
[]


[UserObjects]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1.0E2
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.5E2
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
  [./pgas]
  [../]
  [./swater]
  [../]
[]

[ICs]
  [./pp_ic]
    type = ConstantIC
    value = 1
    variable = pgas
  [../]
  [./sat_ic]
    type = ConstantIC
    value = 0.5
    variable = swater
  [../]
[]


[Kernels]
  [./liquid_mass_dot]
    type = Q2PMassChange
    variable = swater
    other_var = pgas
    var_is_porepressure = false
    fluid_density = DensityWater
  [../]
  [./gas_mass_dot]
    type = Q2PMassChange
    variable = pgas
    other_var = swater
    var_is_porepressure = true
    fluid_density = DensityGas
  [../]
  [./gas_flux]
    type = Q2PPorepressureFlux
    variable = pgas
    fluid_density = DensityGas
    saturation_variable = swater
    fluid_relperm = RelPermGas
    fluid_viscosity = 1
  [../]
  [./water_flux]
    type = Q2PSaturationFlux
    variable = swater
    fluid_density = DensityWater
    porepressure_variable = pgas
    fluid_relperm = RelPermWater
    fluid_viscosity = 1
  [../]
[]

[Postprocessors]
  [./pp_left]
    type = PointValue
    point = '0 0 0'
    variable = pgas
  [../]
  [./pp_right]
    type = PointValue
    point = '1 0 0'
    variable = pgas
  [../]

  [./sat_left]
    type = PointValue
    point = '0 0 0'
    variable = swater
  [../]
  [./sat_right]
    type = PointValue
    point = '1 0 0'
    variable = swater
  [../]
[]


[Materials]
  [./rock]
    type = Q2PMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    gravity = '-1 0 0'
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6

  [./TimeStepper]
    type = FunctionDT
    time_dt = '1E-2 1E-1 1E0 1E1 1E3 1E4 1E5 1E6 1E7'
    time_t = '0 1E-1 1E0 1E1 1E2 1E3 1E4 1E5 1E6'
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ghQ2P_pgas
  csv = true
  exodus = true
[]
