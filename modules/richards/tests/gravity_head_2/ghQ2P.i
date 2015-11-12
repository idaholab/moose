# quick two phase

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
    type = Q2PRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./RelPermGas]
    type = RichardsRelPermPowerGas
    simm = 0.0
    n = 3
  [../]
[]

[Variables]
  [./pp]
  [../]
  [./sat]
  [../]
[]

[ICs]
  [./pp_ic]
    type = ConstantIC
    value = 1
    variable = pp
  [../]
  [./sat_ic]
    type = ConstantIC
    value = 0.5
    variable = sat
  [../]
[]


[Kernels]
  [./liquid_mass_dot]
    type = Q2PMassChange
    variable = pp
    other_var = sat
    var_is_porepressure = true
    fluid_density = DensityWater
  [../]
  [./gas_mass_dot]
    type = Q2PMassChange
    variable = sat
    other_var = pp
    var_is_porepressure = false
    fluid_density = DensityGas
  [../]
  [./liquid_flux]
    type = Q2PPorepressureFlux
    variable = pp
    fluid_density = DensityWater
    saturation_variable = sat
    fluid_relperm = RelPermWater
    fluid_viscosity = 1
  [../]
  [./gas_flux]
    type = Q2PSaturationFlux
    variable = sat
    fluid_density = DensityGas
    porepressure_variable = pp
    fluid_relperm = RelPermGas
    fluid_viscosity = 1
  [../]
[]

[Postprocessors]
  [./pp_left]
    type = PointValue
    point = '0 0 0'
    variable = pp
  [../]
  [./pp_right]
    type = PointValue
    point = '1 0 0'
    variable = pp
  [../]

  [./sat_left]
    type = PointValue
    point = '0 0 0'
    variable = sat
  [../]
  [./sat_right]
    type = PointValue
    point = '1 0 0'
    variable = sat
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
  file_base = ghQ2P
  csv = true
[]
