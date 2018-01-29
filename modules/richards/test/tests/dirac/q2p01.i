# unsaturated
# production
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1E-2 1E-1 1 1E1 1E2 1E3'
    x = '0 1E-1 1 1E1 1E2 1E3'
  [../]
[]


[UserObjects]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 0.5
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 0.5
    bulk_mod = 0.3
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.2
    n = 2
  [../]
  [./RelPermGas]
    type = Q2PRelPermPowerGas
    simm = 0.1
    n = 3
  [../]

  [./borehole_total_outflow_water]
    type = RichardsSumQuantity
  [../]
  [./borehole_total_outflow_gas]
    type = RichardsSumQuantity
  [../]
[]


[Variables]
  [./pp]
  [../]
  [./sat]
  [../]
[]

[ICs]
  [./p_ic]
    type = ConstantIC
    variable = pp
    value = 1
  [../]
  [./s_ic]
    type = ConstantIC
    variable = sat
    value = 0.5
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
[]

[DiracKernels]
  [./bh_water]
    type = Q2PBorehole
    bottom_pressure = 0
    point_file = bh02.bh
    SumQuantityUO = borehole_total_outflow_water
    variable = sat
    unit_weight = '0 0 0'
    character = 8E9
    fluid_density = DensityWater
    fluid_relperm = RelPermWater
    other_var = pp
    var_is_porepressure = false
    fluid_viscosity = 0.8
  [../]
  [./bh_gas]
    type = Q2PBorehole
    bottom_pressure = 0
    point_file = bh02.bh
    SumQuantityUO = borehole_total_outflow_gas
    variable = pp
    unit_weight = '0 0 0'
    character = 1E10
    fluid_density = DensityGas
    fluid_relperm = RelPermGas
    other_var = sat
    var_is_porepressure = true
    fluid_viscosity = 0.5
  [../]
[]



[Postprocessors]
  [./bh_report_water]
    type = RichardsPlotQuantity
    uo = borehole_total_outflow_water
  [../]
  [./bh_report_gas]
    type = RichardsPlotQuantity
    uo = borehole_total_outflow_gas
  [../]

  [./p0]
    type = PointValue
    variable = pp
    point = '1 1 1'
    execute_on = timestep_end
  [../]
  [./sat0]
    type = PointValue
    variable = sat
    point = '1 1 1'
    execute_on = timestep_end
  [../]
[]




[Materials]
  [./rock]
    type = Q2PMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-12 0 0  0 1E-12 0  0 0 1E-12'
    gravity = '0 0 0'
  [../]
[]


[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 30'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 1E3
  solve_type = NEWTON

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]


[]

[Outputs]
  file_base = q2p01
  execute_on = timestep_end
  [./CSV]
    type = CSV
  [../]
[]
