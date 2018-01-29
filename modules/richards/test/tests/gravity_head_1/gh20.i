# investigating validity of immobile saturation
# 5 elements, no SUPG

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
  xmin = -1
  xmax = 1
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1 10 100 1000 10000'
    x = '0 10 100 1000 10000'
  [../]
[]

[UserObjects]
  [./PPNames]
    type = RichardsVarNames
    richards_vars = pressure
  [../]
  [./DensityConstBulk]
    type = RichardsDensityConstBulk
    dens0 = 1
    bulk_mod = 1.0E3
  [../]
  [./SeffVG]
    type = RichardsSeff1VG
    m = 0.8
    al = 1
  [../]
  [./RelPermPower]
    type = RichardsRelPermPower
    simm = 0.3
    n = 2
  [../]
  [./Saturation]
    type = RichardsSat
    s_res = 0.1
    sum_s_res = 0.1
  [../]
  [./SUPGnone]
    type = RichardsSUPGnone
  [../]
[]

[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = -1.0
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

[AuxKernels]
  [./Seff1VG_AuxK]
    type = RichardsSeffAux
    variable = Seff1VG_Aux
    seff_UO = SeffVG
    pressure_vars = pressure
  [../]
[]

[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    SUPG_UO = SUPGnone
    sat_UO = Saturation
    seff_UO = SeffVG
    viscosity = 1E-3
    gravity = '-1 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E0
  end_time = 1E5

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  file_base = gh20
  execute_on = 'timestep_end final'
  interval = 10000
  exodus = true
[]
