# two-phase, fully-saturated
# production
# lumped
[Mesh]
  type = FileMesh
  file = th02_input.e
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityGas'
  relperm_UO = 'RelPermWater RelPermGas'
  SUPG_UO = 'SUPGwater SUPGgas'
  sat_UO = 'SatWater SatGas'
  seff_UO = 'SeffWater SeffGas'
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1 2 4 20'
    x = '0 1 10 100'
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
    p_SUPG = 1E-5
  [../]
  [./SUPGgas]
    type = RichardsSUPGstandard
    p_SUPG = 1E-5
  [../]

  [./total_outflow_mass]
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

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]


[Kernels]
  active = 'richardsfwater richardstwater richardsfgas richardstgas'
  [./richardstwater]
    type = RichardsLumpedMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsLumpedMassChange
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
    type = RichardsPolyLineSink
    pressures = '-1E9 1E9'
    fluxes = '200 200'
    point_file = th01.points
    SumQuantityUO = total_outflow_mass
    variable = pwater
  [../]
[]


[Postprocessors]
  [./flow_report]
    type = RichardsPlotQuantity
    uo = total_outflow_mass
  [../]
  [./p50]
    type = PointValue
    variable = pwater
    point = '50 0 0'
    execute_on = timestep_end
  [../]
[]


[Functions]
  [./initial_pressure]
    type = ParsedFunction
    expression = 1E5
  [../]
[]


[Materials]
  [./all]
    type = RichardsMaterial
    block = 1
    mat_porosity = 0.1
    mat_permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
    viscosity = '1E-3 1E-5'
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]



[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it -ksp_rtol -ksp_atol'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               2E-7 1E-10 20 1E-10 1E-100'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 100
  solve_type = NEWTON

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]


[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = th_lumped_22
  csv = true
[]
