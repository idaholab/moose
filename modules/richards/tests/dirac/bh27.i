2-phase version of bh07 (go to steadystate with borehole)
[Mesh]
  type = FileMesh
  file = bh07_input.e
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
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
    m = 0.8
    al = 1E-5
  [../]
  [./SeffGas]
    type = RichardsSeff2gasVG
    m = 0.8
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
    type = RichardsSUPGstandard
    p_SUPG = 1E8
  [../]
  [./SUPGgas]
    type = RichardsSUPGstandard
    p_SUPG = 1E8
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
    function = initial_pressure
  [../]
  [./gas_ic]
    type = FunctionIC
    variable = pgas
    function = initial_pressure
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
    type = RichardsFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsMassChange
    variable = pgas
  [../]
  [./richardsfgas]
    type = RichardsFlux
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
    variable = pwater
    unit_weight = '0 0 0'
    re_constant = 0.1594
    character = two # this is to make the length=1 borehole fill the entire z=2 height
    MyNameIsAndyWilkins = false
  [../]
[]


[Postprocessors]
  [./bh_report]
    type = RichardsPlotQuantity
    uo = borehole_total_outflow_mass
  [../]

  [./water_mass]
    type = RichardsMass
    variable = pwater
  [../]
  [./gas_mass]
    type = RichardsMass
    variable = pgas
  [../]
[]


[Functions]
  [./initial_pressure]
    type = ParsedFunction
    value = 1E7
  [../]
  [./two]
    type = ConstantFunction
    value = 2
  [../]
[]


[Materials]
  [./all]
    type = RichardsMaterial
    block = 1
    mat_porosity = 0.1
    mat_permeability = '1E-11 0 0  0 1E-11 0  0 0 1E-11'
    density_UO = 'DensityWater DensityGas'
    relperm_UO = 'RelPermWater RelPermGas'
    SUPG_UO = 'SUPGwater SUPGgas'
    sat_UO = 'SatWater SatGas'
    seff_UO = 'SeffWater SeffGas'
    viscosity = '1E-3 1E-5'
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it -ksp_rtol'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 10000 1E-10'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 1000
  solve_type = NEWTON

  [./TimeStepper]
    # get only marginally better results for smaller time steps
    type = FunctionDT
    time_dt = '1000 10000'
    time_t = '100 1000'
  [../]

[]

[Outputs]
  file_base = bh27
  output_initial = true
  interval = 10000
  exodus = true
  print_perf_log = true
[]
