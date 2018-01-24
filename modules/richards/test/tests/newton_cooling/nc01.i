[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1000
  ny = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 1
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
    bulk_mod = 1.0E6
  [../]
  [./SeffVG]
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
    s_res = 0.0
    sum_s_res = 0.0
  [../]
  [./SUPGnone]
    type = RichardsSUPGnone
  [../]
[]




[Variables]
  active = 'pressure'
  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2E6
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 2E6
  [../]
  [./newton]
    type = RichardsPiecewiseLinearSink
    variable = pressure
    boundary = right
    pressures = '0 100000 200000 300000 400000 500000 600000 700000 800000 900000 1000000 1100000 1200000 1300000 1400000 1500000 1600000 1700000 1800000 1900000 2000000'
    bare_fluxes = '0. 5.6677197748570516e-6 0.000011931518841831313 0.00001885408740732065 0.000026504708864284114 0.000034959953203725676 0.000044304443352900224 0.00005463170211001232 0.00006604508815181467 0.00007865883048198513 0.00009259917167338928 0.00010800563134618119 0.00012503240252705603 0.00014384989486488752 0.00016464644014777016 0.00018763017719085535 0.0002130311349595711 0.00024110353477682344 0.00027212833465544285 0.00030641604122040985 0.00034430981736352295'
    use_mobility = false
    use_relperm = false
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



[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-15 0 0  0 1E-15 0  0 0 1E-15'
    density_UO = DensityConstBulk
    relperm_UO = RelPermPower
    SUPG_UO = SUPGnone
    sat_UO = Saturation
    seff_UO = SeffVG
    viscosity = 1E-3
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  active = 'andy'
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-15 10000'
  [../]


[]

[Executioner]
  type = Transient
  end_time = 1E8
  dt = 1E6
[]

[Outputs]
  file_base = nc01
  interval = 100000
  execute_on = 'initial final'
  exodus = true
[]
