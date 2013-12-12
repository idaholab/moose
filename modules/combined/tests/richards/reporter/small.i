[Mesh]
  #type = GeneratedMesh
  #dim = 3
  #nx = 1
  #ny = 1
  #nz = 1
  #xmin = -5E6
  #xmax = 5E6
  #ymin = -5E6
  #ymax = 5E6
  #zmin = -5E6
  #zmax = 5E6
  file = cyl-tet.e
[]

[UserObjects]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2E9
  [../]
  [./Seff1VG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-5
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0
    sum_s_res = 0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1E8
  [../]
  
  [./total_outflow_mass]
    type = RichardsSumQuantity
  [../]
[]


[Variables]
  active = 'pressure'

  [./pressure]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./p_ic]
    type = FunctionIC
    variable = pressure
    function = initial_pressure
  [../]
[]

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]
    
  
[Kernels]
  active = 'richardsf'
  [./richardst]
    type = RichardsMassChange
    variable = pressure
  [../]
  [./richardsf]
    type = RichardsFlux
    variable = pressure
  [../]
[]

[DiracKernels]
  active = 'example_stream'

  [./DAVID_DO_NOT_USE]
    type = RichardsBorehole
    bottom_pressure = 1E6
    point_file = small.bh
    reporter = bh_report
    variable = pressure
    unit_weight = '0 0 -10000'
    well_constant_production = 1.0E-12
    well_constant_injection = 0.0
  [../]

  [./example_stream]
    type = RichardsPolyLineSink
    variable = pressure
    pressures = '0 1E9'
    fluxes = '0 1'
    point_file = small.bh
    reporter = total_outflow_mass
  [../]
[]


[Postprocessors]
  [./bh_report]
    type = Reporter
    sum = true
  [../]

  [./st_report]
    type = RichardsPlotQuantity
    uo = total_outflow_mass
  [../]

  [./fluid_mass0]
    type = RichardsMass
    variable = pressure
    execute_on = timestep_begin
    #output = file
  [../]

  [./fluid_mass1]
    type = RichardsMass
    variable = pressure
    execute_on = timestep
    #output = file
  [../]

  [./zmass_error]
    type = PlotFunction
    function = mass_bal_fcn
    execute_on = timestep
  [../]

  [./delP]
    type = NodalMaxVarChange
    variable = pressure
    execute_on = timestep
  [../]

  [./delS]
    type = NodalMaxVarChange
    variable = Seff1VG_Aux
    execute_on = timestep
  [../]

  [./p0]
    type = PointValue
    variable = pressure
    point = '0 0 0'
    execute_on = timestep
  [../]
[]


[Functions]
 active = 'mass_bal_fcn maxdelp maxdels initial_pressure'

  [./initial_pressure]
    type = ParsedFunction
    value = 1E7-10000*z
  [../]

  [./mass_bal_fcn]
    type = ParsedFunction
    value = abs((a-c+d+f)/2/(a+c))
    vars = 'a c d f'
    vals = 'fluid_mass1 fluid_mass0 bh_report st_report'
  [../]

  [./maxdelp]
    type = ParsedFunction
    value = a
    vars = 'a'
    vals = 'delP'
  [../]

  [./maxdels]
    type = ParsedFunction
    value = a
    vars = 'a'
    vals = 'delS'
  [../]

[]


[Materials]
  [./all]
    type = RichardsMaterial
    block = 1
    viscosity = 0.3
    mat_porosity = 0.1
    mat_permeability = '1E-12 0 0  0 1E-12 0  0 0 1E-12'
    pressure_vars = pressure
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    sat_UO = Saturation
    seff_UO = Seff1VG
    SUPG_UO = SUPGstandard
    gravity = '0 0 -10'
    linear_shape_fcns = true
  [../]
[]

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = Seff1VG
    pressure_vars = pressure
  [../]
[]

[BCs]
  active = ''
  [./top_rain_etc]
    type = RichardsPiecewiseLinearSink
    variable = pressure
    boundary = 'top'
    pressures = '0 1E9'
    fluxes = '0 1'
  [../]

[]

[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 30'
  [../]
[]


[Executioner]
  type = Transient   # Here we use the Transient Executioner
  end_time = 1
  num_steps = 40
  solve_type = NEWTON

  [./TimeStepper]
    type = FunctionControlledDT
    functions = 'mass_bal_fcn maxdelp maxdels'
    maximums = '0.01 1E7 0.5'
    minimums = '1E-4 1E6 0.2'
    dt = 0.1
    increment = 2
    decrement = 0.5
    maxDt = 1E9
    minDt = 0.01
    adapt_log = true
    percent_change = 0.1
  [../]
[]

[Output]
  file_base = small
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
  linear_residuals = true
[]
