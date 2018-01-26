#2-phase version of bh07 (go to steadystate with borehole)
[Mesh]
  type = FileMesh
  file = bh07_input.e
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityGas'
  relperm_UO = 'RelPermWater RelPermGas'
  SUPG_UO = 'SUPGwater SUPGgas'
  sat_UO = 'SatWater SatGas'
  seff_UO = 'SeffWater SeffGas'
  viscosity = '1E-3 1E-5'
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1000 10000'
    x = '100 1000'
  [../]
[]


[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = 'pwater pgas'
  [../]
  [./DensityWater]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E9
  [../]
  [./DensityGas]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 2E6
  [../]
  [./SeffWater]
    type = RichardsSeff2waterVG
    m = 0.6
    al = 1E-5
  [../]
  [./SeffGas]
    type = RichardsSeff2gasVG
    m = 0.6
    al = 1E-5
  [../]
  [./RelPermWater]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./RelPermGas]
    type = RichardsRelPermPower
    simm = 0.0
    n = 3
  [../]
  [./SatWater]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SatGas]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGwater]
    type = RichardsSUPGnone
  [../]
  [./SUPGgas]
    type = RichardsSUPGnone
  [../]

  [./borehole_total_outflow_mass]
    type = RichardsSumQuantity
  [../]
[]

[Variables]
  [./pwater]
    order = FIRST
    family = LAGRANGE
  [../]
  [./pgas]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./water_ic]
    type = FunctionIC
    variable = pwater
    function = 1E7
  [../]
  [./gas_ic]
    type = FunctionIC
    variable = pgas
    function = 1E7
  [../]
[]

[BCs]
  [./fix_outer_w]
    type = DirichletBC
    boundary = perimeter
    variable = pwater
    value = 1E7
  [../]
  [./fix_outer_g]
    type = DirichletBC
    boundary = perimeter
    variable = pgas
    value = 1E7
  [../]
[]

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]


[Kernels]
  active = 'richardsfwater richardstwater richardsfgas richardstgas'
  [./richardstwater]
    type = RichardsMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFullyUpwindFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsMassChange
    variable = pgas
  [../]
  [./richardsfgas]
    type = RichardsFullyUpwindFlux
    variable = pgas
  [../]
[]

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = SeffWater
    pressure_vars = 'pwater pgas'
  [../]
[]

[DiracKernels]
  [./bh]
    type = RichardsBorehole
    bottom_pressure = 0
    point_file = bh07.bh
    SumQuantityUO = borehole_total_outflow_mass
    fully_upwind = true
    variable = pwater
    unit_weight = '0 0 0'
    re_constant = 0.1594
    character = 2 # this is to make the length 1m borehole fill the entire 2m height
  [../]
  [./bh_gas_dummy]
    type = RichardsBorehole
    bottom_pressure = 0
    point_file = bh07.bh
    SumQuantityUO = borehole_total_outflow_mass
    fully_upwind = true
    variable = pgas
    unit_weight = '0 0 0'
    re_constant = 0.1594
    character = 2 # this is to make the length 1m borehole fill the entire 2m height
  [../]
[]


[Postprocessors]
  [./bh_report]
    type = RichardsPlotQuantity
    uo = borehole_total_outflow_mass
    execute_on = 'initial timestep_end'
  [../]

  [./water_mass]
    type = RichardsMass
    variable = pwater
    execute_on = 'initial timestep_end'
  [../]
  [./gas_mass]
    type = RichardsMass
    variable = pgas
    execute_on = 'initial timestep_end'
  [../]
[]



[Materials]
  [./all]
    type = RichardsMaterial
    block = 1
    mat_porosity = 0.1
    mat_permeability = '1E-11 0 0  0 1E-11 0  0 0 1E-11'
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it -ksp_rtol -ksp_atol'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10 1E-10 20 1E-10 1E-100'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 1000
  solve_type = NEWTON

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]

[]

[Outputs]
  file_base = bh27
  execute_on = 'initial timestep_end final'
  interval = 1000000
  exodus = true
[]
