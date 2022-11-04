# fully-saturated
# production
[Mesh]
  type = FileMesh
  file = th02_input.e
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
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
    richards_vars = pressure
  [../]
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
    p_SUPG = 1E-5
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
  active = 'richardsf richardst'
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
  [./bh]
    type = RichardsPolyLineSink
    pressures = '-1E9 1E9'
    fluxes = '200 200'
    point_file = th01.points
    SumQuantityUO = total_outflow_mass
    variable = pressure
  [../]
[]


[Postprocessors]
  [./flow_report]
    type = RichardsPlotQuantity
    uo = total_outflow_mass
  [../]
  [./p50]
    type = PointValue
    variable = pressure
    point = '50 0 0'
    execute_on = 'initial timestep_end'
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
    viscosity = 1E-3
    mat_porosity = 0.1
    mat_permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    sat_UO = Saturation
    seff_UO = Seff1VG
    SUPG_UO = SUPGstandard
    gravity = '0 0 0'
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


[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-6 1E-10 10000 30'
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
  file_base = th02
  csv = true
[]
