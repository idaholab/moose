[Mesh]
  type = GeneratedMesh
  dim = 2
  # very little mesh dependence here
  nx = 120
  ny = 1
  xmin = 0
  xmax = 6
  ymin = 0
  ymax = 0.05
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1E-2 1 10 500 5000 5000'
    x = '0 10 100 1000 10000 100000'
  [../]
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1E3
    bulk_mod = 2E7
  [../]
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.336
    al = 1.43E-4
  [../]
  [./RelPermPower]
    type = RichardsRelPermVG1
    scut = 0.99
    simm = 0.0
    m = 0.336
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGstandard]
    type = RichardsSUPGstandard
    p_SUPG = 1.0E+2
  [../]
[]


[Variables]
  active = 'pressure'
  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = -72620.4
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


[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]


[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = SeffVG
    pressure_vars = pressure
  [../]
[]


[BCs]
  active = 'recharge'
  [./recharge]
    type = RichardsPiecewiseLinearSink
    variable = pressure
    boundary = 'right'
    pressures = '0 1E9'
    bare_fluxes = '-2.315E-3 -2.315E-3'
    use_relperm = false
    use_mobility = false
  [../]
[]


[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.33
    mat_permeability = '0.295E-12 0 0  0 0.295E-12 0  0 0 0.295E-12'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    SUPG_UO = SUPGstandard
    sat_UO = Saturation
    seff_UO = SeffVG
    viscosity = 1.01E-3
    gravity = '-10 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  active = 'andy'

  [./andy]
    type = SMP
    full = true
    petsc_options = ''
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-13 1E-15 10000'
  [../]
[]


[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 359424

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]


[Outputs]
  file_base = rd01
  interval = 100000
  execute_on = 'initial final'
  exodus = true
[]
