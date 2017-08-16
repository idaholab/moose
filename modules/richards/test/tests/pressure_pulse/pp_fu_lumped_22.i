# investigating pressure pulse in 1D with 2 phase
# transient

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  richardsVarNames_UO = PPNames
  density_UO = 'DensityWater DensityGas'
  relperm_UO = 'RelPermWater RelPermGas'
  SUPG_UO = 'SUPGwater SUPGgas'
  sat_UO = 'SatWater SatGas'
  seff_UO = 'SeffWater SeffGas'
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
    p_SUPG = 1E3
  [../]
  [./SUPGgas]
    type = RichardsSUPGstandard
    p_SUPG = 1E3
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
    type = ConstantIC
    value = 2E6
    variable = pwater
  [../]
  [./gas_ic]
    type = ConstantIC
    value = 2E6
    variable = pgas
  [../]
[]


[BCs]
  [./left]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = pwater
  [../]
  [./left_gas]
    type = DirichletBC
    boundary = left
    value = 3E6
    variable = pgas
  [../]
[]

[AuxVariables]
  [./Seff1VG_Aux]
  [../]
[]


[Kernels]
  active = 'richardsfwater richardstwater richardsfgas richardstgas pconstraint'
  [./richardstwater]
    type = RichardsLumpedMassChange
    variable = pwater
  [../]
  [./richardsfwater]
    type = RichardsFullyUpwindFlux
    variable = pwater
  [../]
  [./richardstgas]
    type = RichardsLumpedMassChange
    variable = pgas
  [../]
  [./richardsfgas]
    type = RichardsFullyUpwindFlux
    variable = pgas
  [../]
  [./pconstraint]
    type = RichardsPPenalty
    variable = pgas
    a = 1E-8
    lower_var = pwater
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

[Materials]
  [./rock]
    type = RichardsMaterial
    block = 0
    mat_porosity = 0.1
    mat_permeability = '1E-15 0 0  0 1E-15 0  0 0 1E-15'
    viscosity = '1E-3 1E-5'
    gravity = '0 0 0'
    linear_shape_fcns = true
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_monitor -snes_linesearch_monitor'
    petsc_options_iname = '-pc_factor_shift_type'
    petsc_options_value = 'nonzero'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  dtmin = 1E3
  end_time = 1E4
  l_tol = 1.e-4
  nl_rel_tol = 1.e-7
  nl_max_its = 10
  l_max_its = 20
  line_search = 'none'
[]

[Outputs]
  file_base = pp_fu_lumped_22
  execute_on = 'initial timestep_end final'
  interval = 10000
  exodus = true
  [./console]
    type = Console
    interval = 1
  [../]
[]
