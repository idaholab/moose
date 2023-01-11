[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 15
  xmin = 0
  xmax = 15
[]


[GlobalParams]
  richardsVarNames_UO = PPNames
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1000
    bulk_mod = 2.0E6
  [../]
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1E-4
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.0
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1E-5
  [../]
[]


[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]

[AuxKernels]
  active = 'calculate_seff'
  [./calculate_seff]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = SeffVG
    pressure_vars = pressure
  [../]
[]

[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_pressure
    [../]
  [../]
[]

[BCs]
  active = 'left'
  [./left]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 980000
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


[Functions]
 active = 'initial_pressure'
  [./initial_pressure]
    type = ParsedFunction
    expression = max((1000000-x/5*1000000)-20000,-20000)
  [../]
[]


[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.15
    mat_permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    SUPG_UO = SUPGstandard
    sat_UO = Saturation
    seff_UO = SeffVG
    viscosity = 1E-3
    gravity = '-1 0 0'
    linear_shape_fcns = true
  [../]
[]


[Adaptivity]
  marker = errorfrac
  max_h_level = 3
  [./Indicators]
    [./error]
      type = RichardsFluxJumpIndicator
      variable = pressure
    [../]
  [../]
  [./Markers]
    [./errorfrac]
      type = ErrorFractionMarker
      refine = 0.5
      coarsen = 0.3
      indicator = error
   [../]
  [../]
[]

[Preconditioning]
  active = 'andy'

  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 20'
  [../]

[]

[Executioner]
  type = Transient
  end_time = 50


  [./TimeStepper]
    type = FunctionControlledDT
    functions = ''
    maximums = ''
    minimums = ''
    dt = 0.3
    increment = 1.1
    decrement = 1.1
    maxDt = 0.3
    minDt = 1E-5
    adapt_log = false
    percent_change = 0.1
  [../]

[]

[Outputs]
  file_base = bl01_adapt
  interval = 10000
  exodus = true
[]
